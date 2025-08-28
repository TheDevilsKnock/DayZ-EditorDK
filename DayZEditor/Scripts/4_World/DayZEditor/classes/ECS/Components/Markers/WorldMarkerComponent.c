/**
 * @class WorldMarkerComponent
 * @brief Holds the reference to the in-world 3D marker associated with an entity.
 *
 * A MarkerUpdateSystem will use this component to update the marker's
 * position and visibility in the game world.
 */
class WorldMarkerComponent : EcsComponent
{
    EditorObjectWorldMarker m_Marker;

    void WorldMarkerComponent(EditorObjectWorldMarker marker)
    {
        m_Marker = marker;
    }
}