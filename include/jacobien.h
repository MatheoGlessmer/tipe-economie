#include "economie_model.h"
#include "economie_serialize.h"
#include "market.h"

#ifndef JACOBIEN_H
#define JACOBIEN_H

EcoVector market_to_vect(EcoMarket* market);

EcoMatrix jacobien(EcoMarket* market, EcoReal step);

#endif
