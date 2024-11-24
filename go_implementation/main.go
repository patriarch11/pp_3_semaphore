package main

import (
	"crypto/rand"
	"fmt"
	"math"
	"math/big"
	"sync"
	"time"
)

const TOTAL_PRODUCT_COUNT = 10
const PRODUCERS_COUNT = 3
const CONSUMERS_COUNT = 4
const STORAGE_CAPACITY = 2

const (
	Red   = "\033[31m"
	Green = "\033[32m"
	Blue  = "\033[34m"
	Cyan  = "\033[36m"
	Reset = "\033[0m"
)

type Product struct {
	id string
}

type Strorage = Semaphore[Product]

func NewStorage() *Strorage {
	return NewSemaphore[Product](STORAGE_CAPACITY)
}

type Producer struct {
	id             int
	toProduceCount int
	strorage       *Strorage
}

func (p *Producer) Produce(wg *sync.WaitGroup) {
	fmt.Printf("%s %sВиробник #%d виробить%s %d продуктів\n",
		time.Now().Format("15:04:01.00000"), Red, p.id, Reset, p.toProduceCount,
	)
	defer wg.Done()
	for range p.toProduceCount {
		id := genId()
		p.strorage.Acquire(Product{id: id})
		fmt.Printf("%s %sВирбник #%d відправив%s на склад продукт #%s\n",
			time.Now().Format("15:04:01.00000"), Cyan, p.id, Reset, id,
		)
	}
}

type Consumer struct {
	id         int
	toUseCount int
	storage    *Strorage
}

func (c *Consumer) Use(wg *sync.WaitGroup) {
	fmt.Printf("%s %sСпоживач #%d використає%s %d продуктів\n",
		time.Now().Format("15:04:01.00000"), Green, c.id, Reset, c.toUseCount,
	)
	defer wg.Done()
	for range c.toUseCount {
		product := c.storage.Release()
		fmt.Printf("%s %sСпоживач #%d використав%s продукт #%s\n",
			time.Now().Format("15:04:01.00000"), Blue, c.id, Reset, product.id,
		)
	}
}

func genId() string {
	const charset = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789"
	result := make([]byte, 4)
	for i := range result {
		num, err := rand.Int(rand.Reader, big.NewInt(int64(len(charset))))
		if err != nil {
			return ""
		}
		result[i] = charset[num.Int64()]
	}
	return string(result)
}

func sumList(l []int) int {
	res := 0
	for i := range l {
		res += l[i]
	}
	return res
}

// getJobList визначає скільки продуктів виробить кожен виробник чи використає кожен споживач
func getJobList(productCount int, subjectCount int) []int {
	jobList := make([]int, subjectCount)
	averageCount := int(math.Floor(float64(productCount) / float64(subjectCount)))
	for i := range subjectCount {
		// якщо це останній суб'єкт, то даємо йому залишок к-ті
		if i == subjectCount-1 {
			jobList[i] = productCount - sumList(jobList)
		} else {
			jobList[i] = averageCount
		}
	}
	return jobList
}

func main() {
	storage := NewStorage()
	wg := new(sync.WaitGroup)
	toProduceCounts := getJobList(TOTAL_PRODUCT_COUNT, PRODUCERS_COUNT)
	toUseCounts := getJobList(TOTAL_PRODUCT_COUNT, CONSUMERS_COUNT)

	wg.Add(PRODUCERS_COUNT + CONSUMERS_COUNT)

	go func() {
		for i := range PRODUCERS_COUNT {
			producer := &Producer{
				id:             i,
				toProduceCount: toProduceCounts[i],
				strorage:       storage,
			}
			go producer.Produce(wg)
		}
	}()

	go func() {
		for i := range CONSUMERS_COUNT {
			consumer := &Consumer{
				id:         i,
				toUseCount: toUseCounts[i],
				storage:    storage,
			}
			go consumer.Use(wg)
		}
	}()

	wg.Wait()
}
