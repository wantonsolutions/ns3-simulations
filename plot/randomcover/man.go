package main

import (
	"bufio"
	"fmt"
	"log"
	"math/rand"
	"os"
	"strconv"
)

var nanosecond int = 1000
var milisecond int = 1000

func main() {
	file, err := os.Open("random.dat")
	if err != nil {
		log.Fatal(err)
	}
	defer file.Close()

	d, err := os.Create("d.dat")
	r, err := os.Create("raid.dat")

	scanner := bufio.NewScanner(file)
	for scanner.Scan() {
		line := scanner.Text()
		value, err := strconv.Atoi(line)
		if err != nil {
			log.Fatal()
		}
		var dvalue = 0
		var rvalue = 0
		if value != 44236800 {
			if rand.Int()%20 == 0 {
				dvalue = 44236800
			} else {
				dvalue = value
			}

			if rand.Int()%16 == 0 {
				rvalue = 44236800
			} else {
				rvalue = value
			}
		} else {
			dvalue = value
			rvalue = value
		}

		dvalue = dvalue - (4 * nanosecond * milisecond)
		d.WriteString(fmt.Sprintf("%d\n", dvalue))

		rvalue = rvalue - (3500 * nanosecond)
		r.WriteString(fmt.Sprintf("%d\n", rvalue))

	}

	if err := scanner.Err(); err != nil {
		log.Fatal(err)
	}
	fmt.Println("you fucker")
}
