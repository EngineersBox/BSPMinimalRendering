#pragma once

#include "../texturing/texture.hpp"
#include "../raytracer/Globals.hpp"
#include "../physics/Interaction.hpp"
#include "../id/IDGenerator.hpp"

using namespace std;

template <typename T = int, template <typename> class C = Coordinates>
class IEntityBase {
    public:
        virtual ~IEntityBase(){};
        virtual void render() = 0;
        virtual bool operator==(IEntityBase<T, C>& other);
        inline C<T> getLocation() const {
            return this->location;
        }
        inline Texture getSprite() const {
            return this->sprite;
        }
        inline INTERACTION_TYPE getInteractionType() const {
            return this->interaction_type;
        }
        int_id ent_id = IDGenerator::instance()->next();
    private:
        C<T> location;
        Texture sprite;
        INTERACTION_TYPE interaction_type;
};

template <typename T, template <typename> class C>
bool IEntityBase<T,C>::operator==(IEntityBase<T, C>& other) {
    return (this->location == other.location
        &&  this->sprite == other.sprite
        &&  this->interaction_type == other.interaction_type);
};