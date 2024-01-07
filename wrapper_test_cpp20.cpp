#include <optional>
#include <type_traits>

#define RLBOX_WRAPPER_TYPE std::optional<int>

template <typename T>
struct wrapper_t {
  T m_value;

  inline wrapper_t() requires std::is_trivially_constructible_v<T> = default;

  // inline wrapper_t() requires (!std::is_trivially_constructible_v<T>) = delete;

  template<typename TArg>
  inline explicit(!std::is_convertible_v<TArg, T>) wrapper_t(TArg&& aArg)
  noexcept(std::is_nothrow_constructible_v<T, TArg>)
  requires std::is_constructible_v<T, TArg>
  : m_value(std::forward<TArg>(aArg)){}

  template<typename... TArgs>
  inline wrapper_t(TArgs&&... aArgs)
  noexcept(std::is_nothrow_constructible_v<T, TArgs...>)
  requires std::is_constructible_v<T, TArgs...> && (sizeof...(TArgs) != 1)
  : m_value(std::forward<TArgs>(aArgs)...){}

  inline ~wrapper_t() requires std::is_trivially_destructible_v<T> = default;

  // inline ~wrapper_t() requires (!std::is_trivially_destructible_v<T>) = delete;

  inline ~wrapper_t()
  noexcept(std::is_nothrow_destructible_v<T>)
  requires (!std::is_trivially_destructible_v<T>) { m_value.~T();}
};
///////////////////////////////////////////

struct constructor_Trivial_NoThrow {
  int m;
};

struct constructor_NoTrivial_Nothrow {
  int m;
  constructor_NoTrivial_Nothrow() noexcept {}
};

struct constructor_NoTrivial_Throw {
  int m;
  constructor_NoTrivial_Throw() noexcept(false) {}
};

struct constructor_Complex_Nothrow {
  int mA;
  short mB;
  constructor_Complex_Nothrow(int aA, short aB) noexcept : mA(aA), mB(aB) {}
};

struct constructor_Complex_Throw {
  int mA;
  short mB;
  constructor_Complex_Throw(int aA, short aB) noexcept(false) : mA(aA), mB(aB) {}
};

struct constructor_None {
  int m;
  constructor_None() = delete;
};

///////////////////////////////////////////

struct destructor_Trivial_NoThrow {
  int m;
};

struct destructor_NoTrivial_Nothrow {
  int m;
  ~destructor_NoTrivial_Nothrow() noexcept {}
};

struct destructor_NoTrivial_Throw {
  int m;
  ~destructor_NoTrivial_Throw() noexcept(false) {}
};

struct destructor_None {
  int m;
  destructor_None() = default;
  ~destructor_None() = delete;
};

///////////////////////////////////////////

#include <iostream>
#include <string>

#define MATCH_CHECK(wrapper, trait, T) "  " << #trait << ": " << trait<T> << ", " << (trait<T> == trait<wrapper<T>>? "Match" : "!!!!!!!!!!!NOMATCH!!!!!!") << '\n'

template<typename T>
constexpr bool is_constructible_int_short_v = std::is_constructible_v<T, int&, short&>;

#define printStatus(wrapper, T)                                               \
  std::cout << std::boolalpha << #T << ", " << #wrapper << '<' << #T << ">\n" \
            << MATCH_CHECK(wrapper, std::is_constructible_v, T)               \
            << MATCH_CHECK(wrapper, std::is_trivially_constructible_v, T)     \
            << MATCH_CHECK(wrapper, std::is_nothrow_constructible_v, T)       \
            << MATCH_CHECK(wrapper, is_constructible_int_short_v, T)          \
            << MATCH_CHECK(wrapper, std::is_destructible_v, T)                \
            << MATCH_CHECK(wrapper, std::is_trivially_destructible_v, T)      \
            << MATCH_CHECK(wrapper, std::is_nothrow_destructible_v, T)        \
            << '\n'

int main()
{
  new destructor_None();
  new wrapper_t<destructor_None>();
  // NoDest<destructor_None> nd;
  // NoDest<wrapper_t<destructor_None>> wnd;

  // destructor_None* d = new destructor_None;
  // wrapper_t<destructor_None>* wc;

  printStatus(wrapper_t, constructor_Trivial_NoThrow);
  printStatus(wrapper_t, constructor_NoTrivial_Nothrow);
  printStatus(wrapper_t, constructor_NoTrivial_Throw);
  printStatus(wrapper_t, constructor_Complex_Nothrow);
  printStatus(wrapper_t, constructor_Complex_Throw);
  printStatus(wrapper_t, constructor_None);

  printStatus(wrapper_t, destructor_Trivial_NoThrow);
  printStatus(wrapper_t, destructor_NoTrivial_Nothrow);
  printStatus(wrapper_t, destructor_NoTrivial_Throw);
  printStatus(wrapper_t, destructor_None);
}
