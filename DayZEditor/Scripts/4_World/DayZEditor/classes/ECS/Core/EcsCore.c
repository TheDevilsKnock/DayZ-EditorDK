// A simple typedef for an Entity ID within our ECS.
// Renamed to EcsEntity to avoid conflict with the engine's core Entity class.
typedef int EcsEntity;

/**
 * @class EcsComponent
 * @brief Base class for all Components in our ECS.
 *
 * Components are simple, lightweight data containers. They should not contain
 * any complex logic or behavior. Renamed to avoid engine conflicts.
 */
class EcsComponent
{
}

/**
 * @class EcsSystem
 * @brief Base class for all Systems in our ECS.
 *
 * Systems contain all the logic and behavior. They operate on entities
 * that have a specific set of components. Renamed to avoid potential engine conflicts.
 */
class EcsSystem
{
    protected ECS_World m_World;

    void EcsSystem(ECS_World world)
    {
        m_World = world;
    }

    void ~EcsSystem() { }

    /**
	 * @brief Called once when the system is registered with the SystemManager.
	 */
    void OnStart() { }

    /**
	 * @brief Called once when the system is unregistered.
	 */
    void OnStop() { }

    /**
	 * @brief Called every frame by the SystemManager.
	 * @param deltaTime The time in seconds since the last frame.
	 */
    void OnUpdate(float deltaTime) { }
}