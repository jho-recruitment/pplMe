/**
 *  @file
 *  @brief   Strongly typesafe value wrapper.
 *  @author  j.ho
 */
#ifndef PPLME_LIBPPLMECORE_DISCRIMINATEDVALUE_H_
#define PPLME_LIBPPLMECORE_DISCRIMINATEDVALUE_H_


#include <ostream>
#include <typeinfo>
#include <utility>


namespace pplme {
namespace core {


/**
 *  @tparam  V  The value type to encapsulate.
 *  @tparam  D  The type to use to make this type unique.
 *
 *  @remarks  Since DiscriminatedValue does not provide any operators, it is
 *            not particularly useful as a general purpose encapsulating type
 *            (e.g., for types that one might reasonably expect to be able to
 *            perform arithmetic upon).  It does, however, work quite well for
 *            id types and other types that belong to a particular namespace.
 *
 *  @note  DiscrimatedValue derives from its associated DiscriminatingType
 *         so that ADL can find any names in DiscriminatingType's enclosing
 *         namespace.
 *
 *  Example:
 *  @code
 *  namespace system {
 *
 *  struct SystemIdTag {};
 *
 *  using SystemId = pplme::core::DiscriminatedValue<uint32_t, SystemIdTag>;
 *
 *  bool IsValid(SystemId system_id)
 *  {
 *    return system_id.value() != 0;
 *  }
 *
 *  std::ostream& operator<<(std::ostream& stream, SystemId system_id)
 *  {
 *    return stream << system_id.value();
 *  }
 *
 *  }  // namespace
 *  @endcode
 */
template <typename V, typename D>
class DiscriminatedValue : public D {
 public:
  using ValueType = V;
  using DiscriminatingType = D;

  DiscriminatedValue() : value_() {}
  explicit DiscriminatedValue(ValueType value) : value_(std::move(value)) {}

  ValueType const& value() const { return value_; }
  void set_value(ValueType value) { value_ = std::move(value); }

 private:
  ValueType value_;
};


template <typename V, typename D>
bool operator==(DiscriminatedValue<V, D> const& lhs,
                DiscriminatedValue<V, D> const& rhs)
{
  return lhs.value() == rhs.value();
}


template <typename V, typename D>
bool operator!=(DiscriminatedValue<V, D> const& lhs,
                DiscriminatedValue<V, D> const& rhs)
{
  return !operator==(lhs, rhs);
}


/**
 *  @remarks
 *  Not the finest implementation (not just because we're assuming that
 *  std::type_info::name() returns human-readable sanity); however, it'll
 *  do for a first pass.  Alternative implementations could potentially
 *  look for some sort of Traits<D>::Name or sommat.  In any case, it's always
 *  possible to override by defining a similar function in the same namespace
 *  as the enclosing namespace for D.
 */
template <typename V, typename D>
std::ostream& operator<<(std::ostream& stream,
                         DiscriminatedValue<V, D> const& value)
{
  return stream << "[" << typeid(D).name() << "(" << value.value() << ")]";
}


}  // namespace core
}  // namespace pplme


#endif  // PPLME_LIBPPLMECORE_DISCRIMINATEDVALUE_H_
