/**
 * @class ComponentManager
 * @brief Manages the storage and retrieval of all components.
 */
class ComponentManager
{
    // Map<ComponentTypename, Map<EcsEntity, EcsComponent>>
    private ref map<typename, ref map<EcsEntity, EcsComponent>> m_ComponentStores;

    void ComponentManager()
    {
        m_ComponentStores = new map<typename, ref map<EcsEntity, EcsComponent>>();
    }

    /**
	 * @brief Adds a component to an entity.
	 * @param entity The entity to add the component to.
	 * @param component The EcsComponent instance to add.
	 */
    void AddComponent(EcsEntity entity, EcsComponent component)
    {
        typename componentType = component.Type();
        map<EcsEntity, EcsComponent> store = GetStore(componentType);
        store.Set(entity, component);
    }

    /**
	 * @brief Removes a component of a specific type from an entity.
	 * @param entity The entity to remove the component from.
	 * @param componentType The typename of the component to remove.
	 */
    void RemoveComponent(EcsEntity entity, typename componentType)
    {
        map<EcsEntity, EcsComponent> store = GetStore(componentType);
        if (store.Contains(entity))
        {
            store.Remove(entity);
        }
    }

    /**
	 * @brief Retrieves a component of a specific type for an entity.
	 * @param entity The entity that owns the component.
	 * @param componentType The typename of the component to retrieve.
	 * @return The EcsComponent instance, or null if not found.
	 */
    EcsComponent GetComponent(EcsEntity entity, typename componentType)
    {
        map<EcsEntity, EcsComponent> store = GetStore(componentType);
        return store.Get(entity);
    }

    /**
	 * @brief Checks if an entity has a component of a specific type.
	 * @param entity The entity to check.
	 * @param componentType The typename of the component.
	 * @return True if the entity has the component, false otherwise.
	 */
    bool HasComponent(EcsEntity entity, typename componentType)
    {
        map<EcsEntity, EcsComponent> store = GetStore(componentType);
        return store.Contains(entity);
    }

    /**
	 * @brief Retrieves all components of a given type.
	 * @param componentType The typename of the components to retrieve.
	 * @return A map of EcsEntity IDs to EcsComponent instances.
	 */
    map<EcsEntity, EcsComponent> GetAllComponents(typename componentType)
    {
        return GetStore(componentType);
    }

    /**
	 * @brief Cleans up all components associated with a destroyed entity.
	 * @param entity The entity that was destroyed.
	 */
    void EntityDestroyed(EcsEntity entity)
    {
        foreach (typename componentType, map<EcsEntity, EcsComponent> store : m_ComponentStores)
		{
            if (store.Contains(entity))
            {
                store.Remove(entity);
            }
        }
    }

    /**
	 * @brief Helper to get or create a component store for a specific type.
	 * @param componentType The typename of the component.
	 * @return The map used to store components of the given type.
	 */
    private map<EcsEntity, EcsComponent> GetStore(typename componentType)
    {
        map<EcsEntity, EcsComponent> store;
        if (!m_ComponentStores.Find(componentType, store))
        {
            store = new map<EcsEntity, EcsComponent>();
            m_ComponentStores.Insert(componentType, store);
        }
        return store;
    }
}