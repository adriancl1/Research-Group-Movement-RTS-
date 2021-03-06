
#include "j1App.h"
#include "j1Input.h"
#include "j1Render.h"
#include "j1Animation.h"
#include "Units.h"
#include "p2Log.h"
#include "j1Pathfinding.h"
#include "j1Map.h"
#include "j1EntityManager.h"

Unit::Unit(UNIT_TYPE u_type, fPoint pos, int id): Entity(UNIT, pos), unit_type(u_type), action_type(IDLE), id(id)
{
	int dir = rand() % 8;
	switch (dir)
	{
	case 0:
		direction = NORTH;
		break;
	case 1:
		direction = NORTH_EAST;
		break;
	case 2:
		direction = EAST;
		break;
	case 3:
		direction = SOUTH_EAST;
		break;
	case 4:
		direction = SOUTH;
		break;
	case 5:
		direction = SOUTH_WEST;
		break;
	case 6:
		direction = WEST;
		break;
	case 7:
		direction = NORTH_WEST;
		break;
	}
	switch (u_type)
	{
		//ADD UNIT: IF ANY UNIT IS ADDED ADD CODE HERE:
	case TWOHANDEDSWORDMAN:
		SetHp(60);
		attack = 12;
		SetArmor(1);
		speed = 0.9;
		rate_of_fire = 2;
		range = 1;
		unit_class = INFANTRY;
		unit_radius = 6;
		break;

	case CAVALRYARCHER:
		SetHp(50);
		attack = 6;
		SetArmor(1);
		speed = 1.4;
		rate_of_fire = 2;
		range = 4;
		unit_class = ARCHER;
		unit_radius = 12;
		break;

	case SIEGERAM:
		SetHp(270);
		attack = 4;
		SetArmor(-5);
		speed = 0.6;
		rate_of_fire = 5;
		range = 1;
		unit_class = SIEGE;
		break;

	default:
		LOG("Error UNIT TYPE STATS NULL");
		unit_class = NO_CLASS;
		break;
	}
}

void Unit::Update()
{
	AI();
	Move();
	Draw();
}

void Unit::Move()
{
	if (App->input->GetMouseButtonDown(3) == KEY_DOWN && this->GetEntityStatus() == E_SELECTED)
	{
		this->path_list.clear();
		App->input->GetMousePosition(destination.x, destination.y);
		destination.x -= App->render->camera.x;
		destination.y -= App->render->camera.y;
		if (this->GetPath({ destination.x, destination.y }) != -1)
		{
			//TODO 3: when TODO 3 is done, uncomment this line:
			path_list.pop_front();
			GetNextTile();
			this->action_type = WALK;
			this->moving = true;
		}
		else
		{
			this->moving = false;
			this->action_type = IDLE;
		}
	}

	if (this->moving == true)
	{
		this->SetPosition(GetX() + move_vector.x*speed, GetY() + move_vector.y*speed);

		iPoint unit_world;
		unit_world.x = GetX();
		unit_world.y = GetY();
		//TODO 5: Search the next tile the unit must follow. If the unit has reached its objective, stop him
		//Some help:
		//1- Compare the distance between the temporary objective (path_objective) with unit_world, they don't have to be exactly the same pixel, but their distance must be very low.
		//2- Remember path_objective it's not the final destination, but the next tile, use GetNextTile() in order to know if the path ended, if so, make the unit stop (change "moving" and "action_type")
		if (path_objective.DistanceTo(unit_world) < 3)
		{
			//center the unit to the tile
			this->SetPosition(path_objective.x, path_objective.y);
			if (!GetNextTile())
			{
				moving = false;
				this->action_type = IDLE;
			}
		}
		//--
	}
}

void Unit::AI()
{
}

void Unit::Draw()
{
	SDL_Texture* tex = App->anim->GetTexture(unit_type);
	SDL_Rect rect;
	iPoint pivot;

	App->anim->GetAnimationFrame(rect, pivot, this);

	if (direction == NORTH_EAST || direction == EAST || direction == SOUTH_EAST)
		App->render->Blit(tex, GetX(), GetY(), &rect, SDL_FLIP_HORIZONTAL, pivot.x, pivot.y);
	else
		App->render->Blit(tex, GetX() - pivot.x, GetY() - pivot.y, &rect);

	if (this->GetEntityStatus() == E_SELECTED)
		App->render->DrawCircle(this->GetX() + App->render->camera.x, this->GetY() + App->render->camera.y, this->unit_radius, 255, 255, 255);
}

const DIRECTION Unit::GetDir() const
{
	return direction;
}

const UNIT_TYPE Unit::GetUnitType() const
{
	return unit_type;
}

const ACTION_TYPE Unit::GetActionType() const
{
	return action_type;
}

void Unit::SetAction(const ACTION_TYPE action)
{
	action_type = action;
}

int Unit::GetPath(iPoint dest)
{
	iPoint ori = App->map->WorldToMap(GetX(), GetY());
	iPoint destinat = App->map->WorldToMap(dest.x, dest.y);
	return App->pathfinding->SimpleAstar(ori, destinat, path_list);
}

void Unit::PopFirstPath()
{
	path_list.pop_front();
}

void Unit::AddPath(iPoint new_goal)
{
	path_list.push_back(new_goal);
}

//Makes the unit objective change to the next tile of its path, returns true if found, returns false if path ended.
bool Unit::GetNextTile()
{
	bool ret = true;

	if (path_list.size() == 0)
		return false;

	path_objective = App->map->MapToWorld(path_list.front().x, path_list.front().y);
	path_list.pop_front();

	//TODO 4: calculate the vector (distance) that the unit must move from its position to his objective(path_objective).
	
	move_vector.x = (float)path_objective.x - GetX();
	move_vector.y = (float)path_objective.y - GetY();
	float module = (sqrt(move_vector.x*move_vector.x + move_vector.y * move_vector.y));
	move_vector.x = move_vector.x / module;
	move_vector.y = move_vector.y / module;
	
	//--
	
	iPoint direction_vec;
	direction_vec.x = path_objective.x - GetX();
	direction_vec.y = GetY() - path_objective.y;
	angle = (float)RAD_TO_DEG * atan2(direction_vec.y, direction_vec.x);

	if (angle < 0)
		angle += 360;


	if ((0 <= angle &&  angle <= 22.5) || (337.5 <= angle && angle <= 360))
		this->direction = EAST;

	else if (22.5 <= angle &&  angle <= 67.5)
		this->direction = NORTH_EAST;

	else if (67.5 <= angle &&  angle <= 112.5)
		this->direction = NORTH;

	else if (112.5 <= angle &&  angle <= 157.5)
		this->direction = NORTH_WEST;

	else if (157.5 <= angle &&  angle <= 202.5)
		this->direction = WEST;

	else if (202.5 <= angle &&  angle <= 247.5)
		this->direction = SOUTH_WEST;

	else if (247.5 <= angle &&  angle <= 292.5)
		this->direction = SOUTH;

	else if (292.5 <= angle &&  angle <= 337.5)
		this->direction = SOUTH_EAST;

	else
		this->direction = NO_DIRECTION;

	return ret;
}