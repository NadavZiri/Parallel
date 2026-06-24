// 333150358 - Nadav Ziri
package main

import (
	"math/rand"
	"sync"
	"time"
)

func runZone(zoneID int, orders <-chan Order, tokens chan struct{}, events chan<- Event, seedB int64, wg *sync.WaitGroup) {
	rng := rand.New(rand.NewSource(seedB + int64(zoneID)*2000003))
	var innerWg sync.WaitGroup
	for order := range orders {
		tokens <- struct{}{}
		delay := time.Duration(rng.Intn(21)) * time.Millisecond
		innerWg.Add(1)
		go func(o Order, d time.Duration) {
			defer innerWg.Done()
			events <- Event{Kind: "STARTED", OrderID: o.OrderID, RestaurantID: o.RestaurantID, Zone: o.FoodType}
			time.Sleep(d)
			events <- Event{Kind: "COMPLETED", OrderID: o.OrderID, RestaurantID: o.RestaurantID, Zone: o.FoodType}
			<-tokens
		}(order, delay)
	}
	innerWg.Wait()
	wg.Done()
}
