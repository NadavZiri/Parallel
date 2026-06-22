package main

import "fmt"

func runLogger(events <-chan Event, done chan<- struct{}) {
	counter := 0
    for event := range events {
        switch event.Kind {
        case "CREATED":
            fmt.Printf("CREATED order=%d restaurant=%d type=%d\n", event.OrderID, event.RestaurantID, event.Zone)
        case "DISPATCHED":
            fmt.Printf("DISPATCHED order=%d zone=%d\n", event.OrderID, event.Zone)
		case "STARTED":
			fmt.Printf("STARTED order=%d zone=%d\n", event.OrderID, event.Zone)
		case "COMPLETED":
			fmt.Printf("COMPLETED order=%d zone=%d\n", event.OrderID, event.Zone)
			counter++
		case "DONE":
			fmt.Printf("DONE total=%d\n", counter)
		}
    }
    done <- struct{}{}
}