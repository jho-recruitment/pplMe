/**
 *  @file
 *  Scratch.
 */
#ifndef FOO_BAR_BAZ_H_
#define FOO_BAR_BAZ_H_


#endif  // FOO_BAR_BAZ_H_

#include <stdlib.h>
#include <algorithm>
#include <array>
#include <iostream>
#include <memory>
#include <random>


namespace
{
}

/*
TODO:
Don't forget to multi-thread!
Write overall architecture document (class diagrams and components).
consider using interfaces for Geographical Regions (that way narrowing down lookups can be hidden)...although how is that going to work in terms of investigating the square "surrounding" a particular point?
*/

/**
 *  Represents ...
 */
template <typename DiscriminatorT>
class DecimalDegrees : public DiscriminatorT
{
 public:
  DecimalDegrees() = default;
  explicit constexpr DecimalDegrees(int seconds)
  : seconds_(seconds) {}

  int seconds() const { return seconds_; }
  
  int GetMinutes() const { return seconds_ / 60; }
  
 private:
  int seconds_;
};


struct LatitudeTag {};
using Latitude = DecimalDegrees<LatitudeTag>;

struct LongitudeTag {};
using Longitude = DecimalDegrees<LongitudeTag>;


class Position
{
 public:
  Position() = default;
  Position(Latitude latitude, Longitude longitude)
      : latitude_{latitude}, longitude_{longitude} {}

  Latitude latitude() const { return latitude_; }
  Longitude longitude() const { return longitude_; }
  
 private:
  Latitude latitude_;
  Longitude longitude_;
};



namespace
{
std::default_random_engine g_random_engine;
std::uniform_int_distribution<int> g_random_age(18, 100);
std::uniform_int_distribution<int> g_random_latitude(0 * -90 * 60 * 60, 2 * 90 * 60 * 60);
std::uniform_int_distribution<int> g_random_longitude(0 * -180 * 60 * 60, 2 * 180 * 60 * 60);
}


class Person
{
 public:
  Person(int num) : name_{
    std::string{u8"Baberaham Lincoln ["} + std::to_string(num) + u8"]"}
  {}

  std::string const& name() const { return name_; }
  int age() const { return age_; }
  Position position() const { return position_; }
  
 private:
  std::string name_;
  int age_ = g_random_age(g_random_engine);
  Position position_{Latitude(g_random_latitude(g_random_engine)), Longitude(g_random_longitude(g_random_engine))};
};


class LocationBucket
{
 public:

  void Add(std::unique_ptr<Person> person)
  {
    people_.push_back(std::move(person));
  }

  std::vector<Person> Find(Person const& person)
  {
    std::vector<Person> results;
    for (auto const& candidate : people_)
    {
      if (   person.age() + 5 >= candidate->age()
          && person.age() - 5 <= candidate->age())
      {
        results.push_back(*candidate);
      }
    }
    return results;
  }

 private:
  std::vector<std::unique_ptr<Person>> people_;
};

class Dataset
{
 public:
  Dataset() = default;
  Dataset(Dataset const &) = delete;

  void Add(std::unique_ptr<Person> person)
  {
    auto& bucket = locations_
        [person->position().latitude().GetMinutes()]
        [person->position().longitude().GetMinutes()];
    bucket.Add(std::move(person));
  }

  std::vector<Person> Find(Person const& person)
  {
    return locations_
        [person.position().latitude().GetMinutes()]
        [person.position().longitude().GetMinutes()].
        Find(person);
  }
  
 private:
  std::array<std::array<LocationBucket, 2 * 90 * 60>, 2 * 180 * 60> locations_;
};

  

int main()
{
  std::cout << sizeof(Person) << std::endl;
  std::cout << sizeof(Dataset) << std::endl;

  Dataset* dataset = new Dataset{};

  for (int i = 0; i < 50000000; ++i)
  {
    std::unique_ptr<Person> person(new Person{i});
    dataset->Add(std::move(person));
  }

  std::cout << "Yo..." << std::endl;
  std::cin.get();

  for (;;)
  {
    Person person{0};
    std::cout << person.name() << " (" << person.age() << "): " << person.position().latitude().seconds() << "/" << person.position().longitude().seconds() << std::endl;
    auto const& nearbies = dataset->Find(person);
    for (auto const& nearby : nearbies)
    {
      std::cout << '\t' << nearby.name() << " (" << nearby.age() << "): " << nearby.position().latitude().seconds() << "/" << nearby.position().longitude().seconds() << std::endl;
    }
  }

  return EXIT_SUCCESS;
}
