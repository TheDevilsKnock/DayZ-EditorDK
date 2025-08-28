/**
 * @class TransformComponent
 * @brief Holds position, orientation (YPR), and scale data for an entity.
 */
class TransformComponent : EcsComponent
{
	vector Position;
	vector Orientation; // Yaw, Pitch, Roll
	float Scale;

	void TransformComponent(vector pos, vector ori, float scl = 1.0)
	{
		Position = pos;
		Orientation = ori;
		Scale = scl;
	}
}