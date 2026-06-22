package main

import (
	"math/rand"
	"sync"
)

func runRestaurant(rid int, n int, r int, z int, seedA int64, orders chan<- Order, events chan<- Event, wg *sync.WaitGroup) {
	defer wg.Done()
	startID := rid * n / r
	endID := (rid + 1) * n / r
	rng := rand.New(rand.NewSource(seedA + int64(rid)*1000003))
	for i := startID; i < endID; i++ {
		zone := rng.Intn(z)
		events <- Event{Kind: "CREATED", OrderID: i, RestaurantID: rid, Zone: zone}
		orders <- Order{OrderID: i, RestaurantID: rid, FoodType: zone}
	}
}