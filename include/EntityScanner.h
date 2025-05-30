#ifndef ENTITY_SCANNER_H
#define ENTITY_SCANNER_H

#include <vector>
#include <mutex>
#include "GameData.h"

void* EntityThreadFunc(void* param);

extern std::vector<Actors> EntityList;
extern std::mutex EntityMutex;

std::vector<Actors> GetEntityList();

#endif