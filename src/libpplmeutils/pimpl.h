/**
 *  @file
 *  @brief   Ruthlessly simple pimpl enabler.
 *  @author  j.ho
 */
#ifndef PPLME_LIBPPLMEUTILS_PIMPL_H_
#define PPLME_LIBPPLMEUTILS_PIMPL_H_


namespace pplme {
namespace utils {


/**
 *  @remarks
 *  Class template std::unique_ptr<> is all well and good, but it doesn't
 *  map cv-ness.  Pimpl does.  Enough said, right?
 */
template <typename T>
class Pimpl final {
 public:
  explicit Pimpl(T* p) : p_{p} {};
  ~Pimpl() { delete p_; }

  T* operator->() { return p_; }
  T const* operator->() const { return p_; }
  T volatile* operator->() volatile { return p_; }
  T const volatile* operator->() const volatile { return p_; }
  
 private:
  T* const p_;
};


}  // namespace utils
}  // namespace pplme


#endif  // PPLME_LIBPPLMEUTILS_PIMPL_H_
