//  SuperTux - PneumaticPlatform
//  Copyright (C) 2007 Christoph Sommer <christoph.sommer@2007.expires.deltadevelopment.de>
//
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include "object/pneumatic_platform.hpp"

#include "object/player.hpp"
#include "object/portable.hpp"
#include "supertux/sector.hpp"

PneumaticPlatform::PneumaticPlatform(const ReaderMapping& reader) :
  MovingSprite(reader, "images/objects/platforms/small.sprite", LAYER_OBJECTS, COLGROUP_STATIC),
  master(nullptr),
  slave(nullptr),
  start_y(0),
  offset_y(0),
  speed_y(0),
  contacts()
{
  start_y = get_pos().y;
}

PneumaticPlatform::PneumaticPlatform(PneumaticPlatform* master_) :
  MovingSprite(*master_),
  master(master_),
  slave(this),
  start_y(master_->start_y),
  offset_y(-master_->offset_y),
  speed_y(0),
  contacts()
{
  set_pos(get_pos() + Vector(master->get_bbox().get_width(), 0));
  master->master = master;
  master->slave = this;
}

PneumaticPlatform::~PneumaticPlatform()
{
  if ((this == master) && (master)) {
    slave->master = nullptr;
    slave->slave = nullptr;
  }
  if ((master) && (this == slave)) {
    master->master = nullptr;
    master->slave = nullptr;
  }
  master = nullptr;
  slave = nullptr;
}

HitResponse
PneumaticPlatform::collision(GameObject& other, const CollisionHit& )
{

  // somehow the hit parameter does not get filled in, so to determine (hit.top == true) we do this:
  auto mo = dynamic_cast<MovingObject*>(&other);
  if (!mo) return FORCE_MOVE;
  if ((mo->get_bbox().p2.y) > (m_bbox.p1.y + 2)) return FORCE_MOVE;

  auto pl = dynamic_cast<Player*>(mo);
  if (pl) {
    if (pl->is_big()) contacts.insert(nullptr);
    auto po = pl->get_grabbed_object();
    auto pomo = dynamic_cast<MovingObject*>(po);
    if (pomo) contacts.insert(pomo);
  }

  contacts.insert(&other);
  return FORCE_MOVE;
}

void
PneumaticPlatform::update(float dt_sec)
{
  if (!slave) {
    Sector::get().add<PneumaticPlatform>(this);
    return;
  }
  if (!master) {
    return;
  }
  if (this == slave) {
    offset_y = -master->offset_y;
    m_movement = Vector(0, (start_y + offset_y) - get_pos().y);
  }
  if (this == master) {
    int contact_diff = static_cast<int>(contacts.size()) - static_cast<int>(slave->contacts.size());
    contacts.clear();
    slave->contacts.clear();

    speed_y += (static_cast<float>(contact_diff) * dt_sec) * 12.8f;
    speed_y -= (offset_y * dt_sec * 0.05f);
    speed_y *= 1 - dt_sec;
    offset_y += speed_y * dt_sec * Sector::get().get_gravity();
    if (offset_y < -256) { offset_y = -256; speed_y = 0; }
    if (offset_y > 256) { offset_y = 256; speed_y = -0; }
    m_movement = Vector(0, (start_y + offset_y) - get_pos().y);
  }
}

void
PneumaticPlatform::move_to(const Vector& pos) {
  Vector shift = pos - m_bbox.p1;
  if (this == slave) {
    master->set_pos(master->get_pos() + shift);
  } else if (this == master) {
    slave->set_pos(slave->get_pos() + shift);
  }
  MovingObject::move_to(pos);
  start_y += shift.y;
}

void
PneumaticPlatform::editor_delete() {
  master->remove_me();
  slave->remove_me();
}

void
PneumaticPlatform::after_editor_set() {
  MovingSprite::after_editor_set();
  slave->change_sprite(m_sprite_name);
}

/* EOF */
