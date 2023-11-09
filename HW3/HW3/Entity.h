enum EntityType { PLAYER, TARGET, ENEMY, BASE };

class Entity
{
private:
    bool m_is_active = true;

    // position and tranformation variables
    glm::vec3 m_position;
    glm::mat4 m_model_matrix;

    // physics variables
    glm::vec3 m_velocity;
    glm::vec3 m_acceleration;
    glm::vec3 m_movement;

    // entity specific
    float m_speed;

    EntityType m_entity_type; // type of entity - treat as NAME

    float m_width = 1;
    float m_height = 1;


public:
    // ————— STATIC VARIABLES ————— //
    static const int SECONDS_PER_FRAME = 4;
    static const int    LEFT = 0,
        RIGHT = 1,
        UP = 2,
        DOWN = 3;

    // PHYSICS (collision in all directions)
    bool m_collided_top = false;
    bool m_collided_bottom = false;
    bool m_collided_left = false;
    bool m_collided_right = false;

    // collision detection
    bool is_touching_target = false;
    bool is_touching_enemy = false;

    GLuint m_texture_id;

    // ————— METHODS ————— //
    Entity();

    void update(float delta_time, Entity* player, Entity* collidable_entities, int collidable_entity_count);
    void render(ShaderProgram* program);

    bool const check_collision(Entity* other) const;
    void const check_collision_y(Entity* collidable_entities, int collidable_entity_count);
    void const check_collision_x(Entity* collidable_entities, int collidable_entity_count);

    void move_left() { m_movement.x = -1.0f; };
    void move_right() { m_movement.x = 1.0f; };
    void move_up() { m_movement.y = 1.0f; };
    void move_down() { m_movement.y = -1.0f; };

    void activate() { m_is_active = true; };
    void deactivate() { m_is_active = false; };

    // ————— GETTERS ————— //
    EntityType const get_entity_type()    const { return m_entity_type; };
    glm::vec3  const get_position()       const { return m_position; };
    glm::vec3  const get_movement()       const { return m_movement; };
    glm::vec3  const get_velocity()       const { return m_velocity; };
    glm::vec3  const get_acceleration()   const { return m_acceleration; };
    float      const get_speed()          const { return m_speed; };
    int        const get_width()          const { return m_width; };
    int        const get_height()         const { return m_height; };

    // ————— SETTERS ————— //
    void const set_entity_type(EntityType new_entity_type) { m_entity_type = new_entity_type; };
    void const set_position(glm::vec3 new_position) { m_position = new_position; };
    void const set_movement(glm::vec3 new_movement) { m_movement = new_movement; };
    void const set_velocity(glm::vec3 new_velocity) { m_velocity = new_velocity; };
    void const set_speed(float new_speed) { m_speed = new_speed; };
    void const set_acceleration(glm::vec3 new_acceleration) { m_acceleration = new_acceleration; };
    void const set_width(float new_width) { m_width = new_width; };
    void const set_height(float new_height) { m_height = new_height; };
};