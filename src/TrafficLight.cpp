#include <iostream>
#include <random>
#include "TrafficLight.h"

/* Implementation of class "MessageQueue" */

template <typename T>
T MessageQueue<T>::receive()
{
    // DONE FP.5a : The method receive should use std::unique_lock<std::mutex> and _condition.wait() 
    // to wait for and receive new messages and pull them from the queue using move semantics. 
    // The received object should then be returned by the receive function. 
    std::unique_lock<std::mutex> mutex(_mutex);
    _cond.wait(mutex, [this] { return !_queue.empty(); });
    T msg = std::move(_queue.front());
    _queue.pop_front();
    return msg;
}

template <typename T>
void MessageQueue<T>::send(T &&msg)
{
    // DONE FP.4a : The method send should use the mechanisms std::lock_guard<std::mutex> 
    // as well as _condition.notify_one() to add a new message to the queue and afterwards send a notification.
    std::lock_guard<std::mutex> mutex(_mutex);
    _queue.push_back(std::move(msg));
    _cond.notify_one();
}


/* Implementation of class "TrafficLight" */


TrafficLight::TrafficLight()
{
    _currentPhase = TrafficLightPhase::red;
}

void TrafficLight::waitForGreen()
{
    // DONE FP.5b : add the implementation of the method waitForGreen, in which an infinite while-loop 
    // runs and repeatedly calls the receive function on the message queue. 
    // Once it receives TrafficLightPhase::green, the method returns.
    while(true)
    {
      TrafficLightPhase tlp = _queue.receive();
      if(tlp == TrafficLightPhase::green)
        return;
    }
}

TrafficLightPhase TrafficLight::getCurrentPhase()
{
    return _currentPhase;
}

void TrafficLight::simulate()
{
    // DONE FP.2b : Finally, the private method „cycleThroughPhases“ should be started in a thread when the public method „simulate“ is called. To do this, use the thread queue in the base class. 
    threads.emplace_back(std::thread(&TrafficLight::cycleThroughPhases, this));
}

//virtual function which is executed in a thread
void TrafficLight::cycleThroughPhases()
{
  // DONE FP.2a : Implement the function with an infinite loop that measures the time between two loop cycles 
  // and toggles the current phase of the traffic light between red and green and sends an update method 
  // to the message queue using move semantics. The cycle duration should be a random value between 4 and 6 seconds. 
  // Also, the while-loop should use std::this_thread::sleep_for to wait 1ms between two cycles. 

  /* Init our random generation between 4 and 6 seconds */
  std::random_device rd;
  std::mt19937 eng(rd());
  std::uniform_int_distribution<> distr(4, 6);

  /* Initalize variables */
  int cycle_duration = distr(eng); //Duration of a single simulation cycle in seconds, is randomly chosen
  /* Init stop watch */
  auto last_update = std::chrono::system_clock::now();
  while(true)
  {
    /* Compute time difference to stop watch */
    long time_since_last_update = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now() - last_update).count();

    /* It is time to toggle our traffic light */
    if (time_since_last_update >= cycle_duration)
    {
      if (_currentPhase == TrafficLightPhase::red)
        _currentPhase = TrafficLightPhase::green;
        else
        _currentPhase = TrafficLightPhase::red;

      TrafficLightPhase message = _currentPhase;
      _queue.send(std::move(message));
      /* Reset stop watch for next cycle */
      last_update = std::chrono::system_clock::now();
      /* Randomly choose the cycle duration for the next cycle */
      cycle_duration = distr(eng);
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
  }
}

