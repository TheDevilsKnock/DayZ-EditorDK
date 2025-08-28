/**
 * @class ECS_World
 * @brief The central hub for the ECS architecture.
 *
 * This class owns and provides access to all the managers (Entity, Component, System).
 * It acts as the primary interface for interacting with the ECS.
 */
class ECS_World
{
    private ref EntityManager m_EntityManager;
    private ref ComponentManager m_ComponentManager;
    private ref SystemManager m_SystemManager;

    void ECS_World()
    {
        m_EntityManager = new EntityManager();
        m_ComponentManager = new ComponentManager();
        m_SystemManager = new SystemManager();
    }

    // --- Entity Methods ---
    EcsEntity CreateEntity()
    {
        return m_EntityManager.CreateEntity();
    }

    void DestroyEntity(EcsEntity entity)
    {
        // The order is important: first remove components, then recycle the ID.
        m_ComponentManager.EntityDestroyed(entity);
        m_EntityManager.DestroyEntity(entity);
    }

    // --- Component Methods ---
    void AddComponent(EcsEntity entity, EcsComponent component)
    {
        m_ComponentManager.AddComponent(entity, component);
    }

    void RemoveComponent(EcsEntity entity, typename componentType)
    {
        m_ComponentManager.RemoveComponent(entity, componentType);
    }

    /**
	 * @brief Retrieves a component of a specific type for an entity.
	 * The caller is responsible for casting the returned EcsComponent to the desired type.
	 * @param entity The entity that owns the component.
	 * @param componentType The typename of the component to retrieve.
	 * @return The EcsComponent instance, or null if not found.
	 */
    EcsComponent GetComponent(EcsEntity entity, typename componentType)
    {
        return m_ComponentManager.GetComponent(entity, componentType);
    }

    bool HasComponent(EcsEntity entity, typename componentType)
    {
        return m_ComponentManager.HasComponent(entity, componentType);
    }

    map<EcsEntity, EcsComponent> GetAllComponents(typename componentType)
    {
        return m_ComponentManager.GetAllComponents(componentType);
    }

    // --- System Methods ---
    void RegisterSystem(EcsSystem system)
    {
        m_SystemManager.RegisterSystem(system);
    }

    void UnregisterSystem(EcsSystem system)
    {
        m_SystemManager.UnregisterSystem(system);
    }

    // --- Update Loop ---
    void Update(float deltaTime)
    {
        m_SystemManager.Update(deltaTime);
    }
}

// Global accessor for the ECS World
private static ref ECS_World s_EcsInstance;
static ECS_World GetECS()
{
    if (!s_EcsInstance)
    {
        s_EcsInstance = new ECS_World();
    }
    return s_EcsInstance;
}