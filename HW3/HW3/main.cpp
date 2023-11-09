/**
* Author: Vitoria Tullo
* Assignment: Lunar Lander
* Date due: 2023-11-08, 11:59pm
* I pledge that I have completed this assignment without
* collaborating with anyone else, in conformance with the
* NYU School of Engineering Policies and Procedures on
* Academic Misconduct.
**/

#define GL_SILENCE_DEPRECATION
#define STB_IMAGE_IMPLEMENTATION
#define LOG(argument) std::cout << argument << '\n'
#define GL_GLEXT_PROTOTYPES 1
#define FIXED_TIMESTEP 0.0166666f
#define ENEMY_COUNT 10

#ifdef _WINDOWS
#include <GL/glew.h>
#endif

#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_mixer.h>
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "ShaderProgram.h"
#include "stb_image.h"
#include "cmath"
#include <ctime>
#include <vector>
#include <cstdlib>
#include "Entity.h"

// list of entities in game
struct GameState
{
    Entity* player;
    Entity* platforms;
};

// CONSTS
// window dimensions + viewport
const int WINDOW_WIDTH = 640 * 2,
WINDOW_HEIGHT = 480 * 2;

const int VIEWPORT_X = 0,
VIEWPORT_Y = 0,
VIEWPORT_WIDTH = WINDOW_WIDTH,
VIEWPORT_HEIGHT = WINDOW_HEIGHT;

// background color 
// CHANGE LATER
const float BG_RED = 0.1922f,
BG_BLUE = 0.549f,
BG_GREEN = 0.9059f,
BG_OPACITY = 1.0f;

// shaders
const char V_SHADER_PATH[] = "shaders/vertex_textured.glsl",
F_SHADER_PATH[] = "shaders/fragment_textured.glsl";

// texture filepaths 
const char PLAYER_FILEPATH[] = "korok_player.png",
TARGET_FILEPATH[] = "korok_target.png",
ENEMY_FILEPATH[] = "gloom.png",
BASE_FILEPATH[] = "link.png",
FONT_FILEPATH[] = "font.png";

const int NUMBER_OF_TEXTURES = 1;
const GLint LEVEL_OF_DETAIL = 0;
const GLint TEXTURE_BORDER = 0;

// time for deltatime
const float MILLISECONDS_IN_SECOND = 1000.0;

// physics constants
const float GRAVITY = -2.3f;
const float SLIDE_FORCE = 2.0f;
const float PUSH_FORCE = 2.5f;

// text const
const int FONTBANK_SIZE = 16;

// GLOBAL
// game state and finished status
GameState g_state;
bool game_completed = false;

SDL_Window* g_display_window;
bool g_game_is_running = true;

ShaderProgram g_shader_program;
glm::mat4 g_view_matrix, g_projection_matrix;

float g_previous_ticks = 0.0f;
float g_accumulator = 0.0f;

// helpers
GLuint load_texture(const char* filepath);
void init_platform(Entity& entity, glm::vec3 position,
    EntityType type, GLuint& texture);
void draw_text(ShaderProgram* program, GLuint font_texture_id, std::string text,
    float screen_size, float spacing, glm::vec3 position);
// for game program
void initialise();
void process_input();
void update();
void render();
void shutdown();

// ––––– GAME LOOP ––––– //
int main(int argc, char* argv[])
{
    initialise(); // initailize all game objects and code -- runs ONCE

    while (g_game_is_running)
    {
        process_input(); // get input from player
        update(); // update the game state, run every frame
        render(); // show the game state (after update to show changes in game state)
    }

    shutdown(); // close game safely
    return 0;
}

/*
* Loads a texture to be used for each sprite
* @param filepath, an array of chars that represents the text name 
  of the filepath of the texture for the sprite
*/
GLuint load_texture(const char* filepath)
{
    // Load image file from filepath
    int width, height, number_of_components;
    unsigned char* image = stbi_load(filepath, &width, &height, &number_of_components, STBI_rgb_alpha);

    // Throw error if no image found at filepath
    if (image == NULL)
    {
        LOG(" Unable to load image. Make sure the path is correct.");
        assert(false);
    }

    // Generate and bind texture ID to image
    GLuint textureID;
    glGenTextures(NUMBER_OF_TEXTURES, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, LEVEL_OF_DETAIL, GL_RGBA, width, height, TEXTURE_BORDER, GL_RGBA, GL_UNSIGNED_BYTE, image);

    // Setting up texture filter parameters
    // NEAREST better for pixel art
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    // Release from memory and return texture id
    stbi_image_free(image);

    return textureID;
}

/*
* Takes all the needed values of a PLATFORM entity and initializes it
* Makes sure the update method for the PLATFORM is set correctly
* 
* @param entity, the ENTITY OBJECT that's being initialized
* @param position, the vec3 the platform starts at
* @param type, ENTITYTYPE OBJECT that determines what type of platform it is
* @param texture, the GLuint displayed texture of the platform
*/
void init_platform(Entity& entity, glm::vec3 position, 
    EntityType type, GLuint& texture)
{
    entity.m_texture_id = texture;
    entity.set_position(position);
    entity.set_entity_type(type);
    entity.update(0.0f, g_state.player, NULL, 0);
}

// initialises all game objects and code
// RUNS ONLY ONCE AT THE START
void initialise()
{
    // create window
    SDL_Init(SDL_INIT_VIDEO);
    g_display_window = SDL_CreateWindow("HW 3!!!!!!",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        WINDOW_WIDTH, WINDOW_HEIGHT,
        SDL_WINDOW_OPENGL);

    SDL_GLContext context = SDL_GL_CreateContext(g_display_window);
    SDL_GL_MakeCurrent(g_display_window, context);

    // for windows machines
#ifdef _WINDOWS
    glewInit();
#endif

    glViewport(VIEWPORT_X, VIEWPORT_Y, VIEWPORT_WIDTH, VIEWPORT_HEIGHT);
    g_shader_program.load(V_SHADER_PATH, F_SHADER_PATH);

    g_view_matrix = glm::mat4(1.0f);
    g_projection_matrix = glm::ortho(-5.0f, 5.0f, -3.75f, 3.75f, -1.0f, 1.0f);

    g_shader_program.set_projection_matrix(g_projection_matrix);
    g_shader_program.set_view_matrix(g_view_matrix);

    glUseProgram(g_shader_program.get_program_id());

    glClearColor(BG_RED, BG_BLUE, BG_GREEN, BG_OPACITY);

    // TEXTURE IDS
    GLuint player_texture_id = load_texture(PLAYER_FILEPATH);
    GLuint target_texture_id = load_texture(TARGET_FILEPATH);
    GLuint enemy_texture_id = load_texture(ENEMY_FILEPATH);
    GLuint base_texture_id = load_texture(BASE_FILEPATH);

    g_state.platforms = new Entity[ENEMY_COUNT + 2]; // account for target and base

    // Set the type of every platform entity to PLATFORM
    for (int i = 0; i < ENEMY_COUNT; i++)
    {
        init_platform(g_state.platforms[i], glm::vec3(i - 4.5f, -3.5f, 0.0f), 
            ENEMY, enemy_texture_id);
    }

    // BASE PLATFORM
    init_platform(g_state.platforms[ENEMY_COUNT], glm::vec3(-4.0f, -2.0f, 0.0f), 
        BASE, base_texture_id);

    // TARGET PLATFORM
    init_platform(g_state.platforms[ENEMY_COUNT + 1], 
        glm::vec3(4.0f, 2.0f, 0.0f), TARGET, target_texture_id);

    // PLAYER
    g_state.player = new Entity();
    g_state.player->m_texture_id = player_texture_id;
    g_state.player->set_position(glm::vec3(-4.0f, 0.0f, 0.0f));
    g_state.player->set_movement(glm::vec3(0.0f));
    g_state.player->set_speed(1.0f);
    g_state.player->set_acceleration(glm::vec3(0.0f, GRAVITY, 0.0f));
    g_state.player->set_entity_type(PLAYER);

    // ––––– GENERAL ––––– //
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

// Reads all the inputs on player's machine
// This includes keyboard, mouse, and window close
void process_input()
{
    // reset player movement vector
    g_state.player->set_movement(glm::vec3(0.0f));

    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        switch (event.type) {
            // check if game is quit
        case SDL_QUIT:
        case SDL_WINDOWEVENT_CLOSE:
            g_game_is_running = false;
            break;

        default:
            break;
        }
    }

    const Uint8* key_state = SDL_GetKeyboardState(NULL);

    // read keyboard inputs while player hasn't touched anything
    if (!game_completed)
    {
        // if there is no movement direction in the y
        // then y-acceleration can be reset to GRAVITY
        glm::vec3 pre_input_movement = g_state.player->get_movement();
        if (pre_input_movement.y == 0)
        {
            g_state.player->set_acceleration(glm::vec3(0.0f, GRAVITY, 0.0f));
        }

        // change movement direction
        // add acceleration in the right direction
        if (key_state[SDL_SCANCODE_LEFT])
        {
            g_state.player->move_left();
            g_state.player->set_acceleration(glm::vec3(-SLIDE_FORCE, 0.0f, 0.0f));
        }
        else if (key_state[SDL_SCANCODE_RIGHT])
        {
            g_state.player->move_right();
            g_state.player->set_acceleration(glm::vec3(SLIDE_FORCE, 0.0f, 0.0f));
        }
        else if (key_state[SDL_SCANCODE_UP])
        {
            g_state.player->move_up();
            g_state.player->set_acceleration(glm::vec3(0.0f, PUSH_FORCE, 0.0f));
        }

        // get the state of the player post input
        glm::vec3 current_velocity = g_state.player->get_velocity();
        glm::vec3 current_acceleration = g_state.player->get_acceleration();
        glm::vec3 post_input_movement = g_state.player->get_movement();

        // if no left and right input 
        // and our player is still moving
        // add a frictional force
        // REMEMBER TO KEEP GRAVITY IN Y -- WILL LEAD TO BUGS OTHERWISE
        if (current_velocity.x > 0 && (post_input_movement.x == 0))
        {
            g_state.player->set_acceleration(glm::vec3(-1.0f, current_acceleration.y, 0.0f));
        }
        else if (current_velocity.x < 0 && (post_input_movement.x == 0))
        {
            g_state.player->set_acceleration(glm::vec3(1.0f, current_acceleration.y, 0.0f));
        }

        if (glm::length(g_state.player->get_movement()) > 1.0f)
        {
            g_state.player->set_movement(
                glm::normalize(
                    g_state.player->get_movement()
                )
            );
        }
    }
}

void update()
{
    // calculate time
    float ticks = (float)SDL_GetTicks() / MILLISECONDS_IN_SECOND; // get the current number of ticks
    float delta_time = ticks - g_previous_ticks; // the delta time is the difference from the last frame
    g_previous_ticks = ticks;

    delta_time += g_accumulator;

    if (delta_time < FIXED_TIMESTEP)
    {
        g_accumulator = delta_time;
        return;
    }

    while (delta_time >= FIXED_TIMESTEP)
    {
        if (!game_completed)
            g_state.player->update(FIXED_TIMESTEP, g_state.player, g_state.platforms, ENEMY_COUNT + 2);
        delta_time -= FIXED_TIMESTEP;
    }

    g_accumulator = delta_time;
}

void render()
{
    glClear(GL_COLOR_BUFFER_BIT);

    // single render
    g_state.player->render(&g_shader_program);

    // batch render -- account for target and base again
    for (int i = 0; i < ENEMY_COUNT + 2; i++) g_state.platforms[i].render(&g_shader_program);

    // if the player object's flag for colliding with an object is true
    // display correct text and stop ALL processes
    if (g_state.player->is_touching_enemy == true)
    {
        GLuint font_texture_id = load_texture(FONT_FILEPATH);
        draw_text(&g_shader_program, font_texture_id, "mission failed", 0.5f,
            -0.2f, glm::vec3(-1.75f, 0.0f, 0.0f));
        game_completed = true;
    }
    if (g_state.player->is_touching_target == true)
    {
        GLuint font_texture_id = load_texture(FONT_FILEPATH);
        draw_text(&g_shader_program, font_texture_id, "mission accomplished", 0.5f,
            -0.2f, glm::vec3(-2.5f, 0.0f, 0.0f));
        game_completed = true;
    }

    SDL_GL_SwapWindow(g_display_window);
}

// shutdown safely
void shutdown()
{
    SDL_Quit();

    // free from memory
    delete[] g_state.platforms;
    delete g_state.player;
}

// provided by professor
void draw_text(ShaderProgram* program, GLuint font_texture_id, std::string text, float screen_size, float spacing, glm::vec3 position)
{
    // Scale the size of the fontbank in the UV-plane
    // We will use this for spacing and positioning
    float width = 1.0f / FONTBANK_SIZE;
    float height = 1.0f / FONTBANK_SIZE;

    // Instead of having a single pair of arrays, we'll have a series of pairs—one for each character
    // Don't forget to include <vector>!
    std::vector<float> vertices;
    std::vector<float> texture_coordinates;

    // For every character...
    for (int i = 0; i < text.size(); i++) {
        // 1. Get their index in the spritesheet, as well as their offset (i.e. their position
        //    relative to the whole sentence)
        int spritesheet_index = (int)text[i];  // ascii value of character
        float offset = (screen_size + spacing) * i;

        // 2. Using the spritesheet index, we can calculate our U- and V-coordinates
        float u_coordinate = (float)(spritesheet_index % FONTBANK_SIZE) / FONTBANK_SIZE;
        float v_coordinate = (float)(spritesheet_index / FONTBANK_SIZE) / FONTBANK_SIZE;

        // 3. Inset the current pair in both vectors
        vertices.insert(vertices.end(), {
            offset + (-0.5f * screen_size), 0.5f * screen_size,
            offset + (-0.5f * screen_size), -0.5f * screen_size,
            offset + (0.5f * screen_size), 0.5f * screen_size,
            offset + (0.5f * screen_size), -0.5f * screen_size,
            offset + (0.5f * screen_size), 0.5f * screen_size,
            offset + (-0.5f * screen_size), -0.5f * screen_size,
            });

        texture_coordinates.insert(texture_coordinates.end(), {
            u_coordinate, v_coordinate,
            u_coordinate, v_coordinate + height,
            u_coordinate + width, v_coordinate,
            u_coordinate + width, v_coordinate + height,
            u_coordinate + width, v_coordinate,
            u_coordinate, v_coordinate + height,
            });
    }

    // 4. And render all of them using the pairs
    glm::mat4 model_matrix = glm::mat4(1.0f);
    model_matrix = glm::translate(model_matrix, position);

    program->set_model_matrix(model_matrix);
    glUseProgram(program->get_program_id());

    glVertexAttribPointer(program->get_position_attribute(), 2, GL_FLOAT, false, 0, vertices.data());
    glEnableVertexAttribArray(program->get_position_attribute());
    glVertexAttribPointer(program->get_tex_coordinate_attribute(), 2, GL_FLOAT, false, 0, texture_coordinates.data());
    glEnableVertexAttribArray(program->get_tex_coordinate_attribute());

    glBindTexture(GL_TEXTURE_2D, font_texture_id);
    glDrawArrays(GL_TRIANGLES, 0, (int)(text.size() * 6));

    glDisableVertexAttribArray(program->get_position_attribute());
    glDisableVertexAttribArray(program->get_tex_coordinate_attribute());
}