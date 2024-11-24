package main

/*
Оскільки в go нема вбудованої реалізації семафора,
реалізуємо її.
*/

// Semaphore на основі каналу з generic типами для передачі даних
type Semaphore[T any] struct {
	ch chan T
}

// NewSemaphore створює новий семафор з розміром size та generic типом T
// вбудована реалізація буферизованого каналу допоможе правильно реалізувати роботу семафору
func NewSemaphore[T any](size int) *Semaphore[T] {
	return &Semaphore[T]{make(chan T, size)}
}

// Acquire захоплює семафор
func (s *Semaphore[T]) Acquire(item T) {
	s.ch <- item
}

// Release звільняє семафор
func (s *Semaphore[T]) Release() T {
	return <-s.ch
}
