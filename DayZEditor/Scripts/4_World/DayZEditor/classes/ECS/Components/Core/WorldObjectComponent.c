/**
 * @class WorldObjectComponent
 * @brief Holds the reference to the actual game world Object associated with an entity.
 */
class WorldObjectComponent : EcsComponent
{
	Object WorldObject;

	void WorldObjectComponent(Object obj)
	{
		WorldObject = obj;
	}
}