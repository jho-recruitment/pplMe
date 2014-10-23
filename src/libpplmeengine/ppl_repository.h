/**
 *  @file
 *  @brief   Definition of the PplRepository interface, which is responsible
 *           for providing an abstraction for in-memory ppl population.
 *  @author  j.ho
 */
#ifndef PPLME_LIBPPLMEENGINE_PPLREPOSITORY_H_
#define PPLME_LIBPPLMEENGINE_PPLREPOSITORY_H_


namespace pplme {
namespace core {
class Person;
}  // core
}  // pplme


namespace pplme {
namespace engine {


/**
 *  @note
 *  Clients of this interface should feel free to ignore synchronization
 *  (i.e., any necessary synchronization should be assumed to be undertaken
 *  by the implementation).
 */
class PplRepository {
 public:
  virtual ~PplRepository() = default;

  /**
   *  Stores the specified @a person in memory.  It is unspecified whether
   *  duplicates (i.e., instances with the same PersonId) clobber each other.
   *
   *  @remarks
   *  Ensuring PersonId-based uniqueness would clearly be desirable in a real
   *  live system, but it is not required right here right now as this interface
   *  is just about populating in-memory datasets.
   */
  virtual void AddPerson(std::unique_ptr<core::Person> person) = 0;
};


}  // namespace engine
}  // namespace pplme


#endif  // PPLME_LIBPPLMEENGINE_PPLREPOSITORY_H_
