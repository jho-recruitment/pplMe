/**
 *  @file
 *  @brief   Strongly typesafe value wrapper.
 *  @author  j.ho
 */
#ifndef PPLME_LIBPPLMECORE_DISCRIMINATEDVALUE_H_
#define PPLME_LIBPPLMECORE_DISCRIMINATEDVALUE_H_


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


}  // namespace core
}  // namespace pplme


#endif  // PPLME_LIBPPLMECORE_DISCRIMINATEDVALUE_H_
