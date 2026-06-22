package main

import (
	"flag"
	"strconv"
	"strings"
	"sync"
)

func main() {
	n := flag.Int("n", 0, "total orders")
	restaurants := flag.Int("restaurants", 0, "total restaurants")
	zones := flag.Int("zones", 0, "total zones")
	tokensStr := flag.String("tokens", "", "tokens per zone")

	seedA := flag.Int64("seedA", 0, "seed A")
	seedB := flag.Int64("seedB", 0, "seed B")
	flag.Parse()

	parts := strings.Split(*tokensStr, ",")
	tokensCount := make([]int, len(parts))
	for i, p := range parts {
		val, _ := strconv.Atoi(p)
		tokensCount[i] = val
	}
	events := make(chan Event)
	orders := make(chan Order)
	zoneChans := make([]chan Order, *zones)
	for i:= range zoneChans {
		zoneChans[i] = make(chan Order)
	}
	tokens := make([]chan struct{}, *zones)
	for i := range tokens {
		tokens[i] = make(chan struct{}, tokensCount[i])
	}

	done := make(chan struct{})
	go runLogger(events, done)

	var restaurantWg sync.WaitGroup
	restaurantWg.Add(*restaurants)
	for rid := 0; rid < *restaurants; rid++ {
		go runRestaurant(rid, *n, *restaurants, *zones, *seedA, orders, events, &restaurantWg)
	}
	go func() {
		restaurantWg.Wait()
		close(orders)
	}()

	zonesChanSend := make([]chan<- Order, *zones)
	for i := range zoneChans {
		zonesChanSend[i] = zoneChans[i]
	}
	go runDispatcher(orders, zonesChanSend, events)

	var wg sync.WaitGroup
	wg.Add(*zones)
	for i := 0; i < *zones; i++ {
		go runZone(i, zoneChans[i], tokens[i], events, *seedB, &wg)
	}

	go func() {
		wg.Wait()
		events <- Event{Kind: "DONE"}
		close(events)
	}()

	<-done
}