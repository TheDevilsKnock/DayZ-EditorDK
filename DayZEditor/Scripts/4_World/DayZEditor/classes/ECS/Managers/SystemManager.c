/**
 * @class SystemManager
 * @brief Manages the registration, unregistration, and execution of all systems.
 */
class SystemManager
{
    private ref array<ref EcsSystem> m_Systems;

    void SystemManager()
    {
        m_Systems = new array<ref EcsSystem>();
    }

    /**
	 * @brief Registers a new system and calls its OnStart method.
	 * @param system The EcsSystem instance to register.
	 */
    void RegisterSystem(EcsSystem system)
    {
        if (!system) return;

        m_Systems.Insert(system);
        system.OnStart();
    }

    /**
	 * @brief Unregisters a system and calls its OnStop method.
	 * @param system The EcsSystem instance to unregister.
	 */
    void UnregisterSystem(EcsSystem system)
    {
        int index = m_Systems.Find(system);
        if (index != -1)
        {
            system.OnStop();
            m_Systems.Remove(index);
        }
    }

    /**
	 * @brief Calls the OnUpdate method on all registered systems.
	 * @param deltaTime The time in seconds since the last frame.
	 */
    void Update(float deltaTime)
    {
        for (int i = 0; i < m_Systems.Count(); i++)
        {
            m_Systems.Get(i).OnUpdate(deltaTime);
        }
    }
}