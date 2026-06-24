package main

func runDispatcher(orders <- chan Order, zones []chan <- Order, events chan <- Event) {
	for order := range orders {
		zones[order.FoodType] <- order
		events <- Event{Kind: "DISPATCHED", OrderID: order.OrderID, RestaurantID: order.RestaurantID, Zone: order.FoodType}
	}
	for _, z := range zones {
		close(z)
	}
}