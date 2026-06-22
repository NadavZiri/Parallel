// 211388921 - Nadav Ziri
package main

type Order struct {
	OrderID int 
	RestaurantID int
	FoodType int
}

type Event struct {
	Kind string
	OrderID int
	RestaurantID int
	Zone int
}