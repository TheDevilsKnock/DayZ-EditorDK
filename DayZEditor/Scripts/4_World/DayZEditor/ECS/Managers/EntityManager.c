/**
 * @class EntityManager
 * @brief Manages the creation, destruction, and recycling of entity IDs.
 */
class EntityManager
{
    private int m_NextEntityID;
    private ref array<EcsEntity> m_AvailableEntities;
    private int m_ActiveEntityCount;

    void EntityManager()
    {
        m_NextEntityID = 1; // Start from 1, so 0 can be considered an invalid/null entity.
        m_AvailableEntities = new array<EcsEntity>();
        m_ActiveEntityCount = 0;
    }

    /**
	 * @brief Creates a new entity ID.
	 * @return The new unique entity ID.
	 */
    EcsEntity CreateEntity()
    {
        EcsEntity newEntity;

        if (m_AvailableEntities.Count() > 0)
        {
            // Reuse an old ID from the pool of available entities.
            newEntity = m_AvailableEntities.Get(0);
            m_AvailableEntities.Remove(0);
        }
        else
        {
            // Generate a brand new ID.
            newEntity = m_NextEntityID;
            m_NextEntityID++;
        }

        m_ActiveEntityCount++;
        return newEntity;
    }

    /**
	 * @brief Destroys an entity ID, making it available for reuse.
	 * @param entity The entity ID to destroy.
	 */
    void DestroyEntity(EcsEntity entity)
    {
        m_AvailableEntities.Insert(entity);
        m_ActiveEntityCount--;
    }

    /**
	 * @brief Gets the current number of active entities.
	 * @return The count of active entities.
	 */
    int GetActiveEntityCount()
    {
        return m_ActiveEntityCount;
    }
}