package main

import "sync"

// fixed-width scalar type(s) with atomic access methods (protected via mutex)
type Atomic struct {
	*sync.Mutex
	bool
	rune
	int64
	uint64
	float32
	float64
}

func MakeAtomic(val ...interface{}) (*Atomic, bool) {
	var a Atomic
	a.Mutex = &sync.Mutex{}
	for _, v := range val {
		// no need for atomic since this object is not yet visible anywhere else
		if !a.nonatomicSet(v) {
			return nil, false // return nil if even 1 unsupported type was provided
		}
	}
	return &a, len(val) > 0 // must receive at least 1 (supported) scalar value
}

// don't use this method directly -- its only intended for use internally when
// initializing the atomic members all at once. also used by the wrapper method
// (which you SHOULD use), since it is protected by the mutex.
func (a *Atomic) nonatomicSet(val interface{}) bool {
	switch val.(type) {
	case bool:
		a.bool = val.(bool)
	case rune:
		a.rune = val.(rune)
	case int64:
		a.int64 = val.(int64)
	case uint64:
		a.uint64 = val.(uint64)
	case float32:
		a.float32 = val.(float32)
	case float64:
		a.float64 = val.(float64)
	default:
		return false
	}
	return true
}

// common mutator method shared by all supported types. returns true unless an
// unrecognized member type was provided.
func (a *Atomic) Set(val interface{}) bool {
	a.Mutex.Lock()
	defer a.Mutex.Unlock()
	return a.nonatomicSet(val)
}

func (a *Atomic) Bool() bool {
	a.Mutex.Lock()
	defer a.Mutex.Unlock()
	return a.bool
}

func (a *Atomic) Rune() rune {
	a.Mutex.Lock()
	defer a.Mutex.Unlock()
	return a.rune
}

func (a *Atomic) Int64() int64 {
	a.Mutex.Lock()
	defer a.Mutex.Unlock()
	return a.int64
}

func (a *Atomic) Uint64() uint64 {
	a.Mutex.Lock()
	defer a.Mutex.Unlock()
	return a.uint64
}

func (a *Atomic) Float32() float32 {
	a.Mutex.Lock()
	defer a.Mutex.Unlock()
	return a.float32
}

func (a *Atomic) Float64() float64 {
	a.Mutex.Lock()
	defer a.Mutex.Unlock()
	return a.float64
}
