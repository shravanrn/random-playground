#include <optional>
#include <type_traits>

#define RLBOX_WRAPPER_TYPE std::optional<int>

namespace detail
{
    template<class T>
    auto test_returnable(int) -> decltype(
        void(static_cast<T(*)()>(nullptr)), std::true_type{}
    );
    template<class>
    auto test_returnable(...) -> std::false_type;

    template<class To, class... From>
    auto test_implicitly_convertible(int) -> decltype(
        void(std::declval<void(&)(To)>()({std::declval<From>() ...})), std::true_type{}
    );
    template<class, class...>
    auto test_implicitly_convertible(...) -> std::false_type;
} // namespace detail

template<class To, class... From>
struct is_convertible_from_many : std::integral_constant<bool,
    (decltype(detail::test_returnable<To>(0))::value &&
     decltype(detail::test_implicitly_convertible<To, From...>(0))::value) ||
    ((std::is_void<From>::value && ...) && std::is_void<To>::value)
> {};

template<class To, class... From>
constexpr bool is_convertible_from_many_v = is_convertible_from_many<To, From...>::value;

#define FORWARDING_CONSTRUCTORS(name, target, TT)                                                                              \
template <class... U, std::enable_if_t<std::is_constructible_v<TT, U...> && is_convertible_from_many_v<TT, U...>>* = nullptr>  \
name(U&&... u) noexcept(std::is_nothrow_constructible_v<TT, U...>): target(std::forward<U>(u)...) {}                           \
                                                                                                                               \
template <class... U, std::enable_if_t<std::is_constructible_v<TT, U...> && !is_convertible_from_many_v<TT, U...>>* = nullptr> \
explicit name(U&&... u) noexcept(std::is_nothrow_constructible_v<TT, U...>): target(std::forward<U>(u)...) {}                  \
                                                                                                                               \
static_assert(true, "Require semi colon")

// The storage base manages the actual storage.and correctly propagates trivial
// construction and destruction from T.
template <typename T,
          bool = std::is_trivially_constructible_v<T>,
          bool = std::is_trivially_destructible_v<T>,
          bool = std::is_destructible_v<T>>
struct optional_storage_base;

template <typename T>
struct optional_storage_base<T, false, false, true> {
  T m_value;
  FORWARDING_CONSTRUCTORS(optional_storage_base, m_value, T);
  ~optional_storage_base() noexcept(std::is_nothrow_destructible_v<T>) { m_value.~T(); }
};

template <typename T>
struct optional_storage_base<T, false, true, true> {
  T m_value;
  FORWARDING_CONSTRUCTORS(optional_storage_base, m_value, T);
  ~optional_storage_base() = default;
};

template <typename T>
struct optional_storage_base<T, true, false, true> {
  T m_value;
  optional_storage_base() = default;
  FORWARDING_CONSTRUCTORS(optional_storage_base, m_value, T);
  ~optional_storage_base() noexcept(std::is_nothrow_destructible_v<T>) { m_value.~T(); }
};

template <typename T>
struct optional_storage_base<T, true, true, true> {
  T m_value;
  optional_storage_base() = default;
  FORWARDING_CONSTRUCTORS(optional_storage_base, m_value, T);
  ~optional_storage_base() = default;
};

template <typename T>
struct optional_storage_base<T, true, false, false> {
  T m_value;
  optional_storage_base() = default;
  FORWARDING_CONSTRUCTORS(optional_storage_base, m_value, T);
  ~optional_storage_base() = delete;
};

template <typename T>
struct optional_storage_base<T, false, false, false> {
  T m_value;
  FORWARDING_CONSTRUCTORS(optional_storage_base, m_value, T);
  ~optional_storage_base() = delete;
};

///////////////////////////////////////////

template <typename T,
          bool = std::is_copy_constructible_v<T>,
          bool = std::is_move_constructible_v<T>>
struct delete_copymove_constructor_base;

template <typename T>
struct delete_copymove_constructor_base<T, true, true> {
  delete_copymove_constructor_base() = default;

  delete_copymove_constructor_base(const delete_copymove_constructor_base &) = default;
  delete_copymove_constructor_base(delete_copymove_constructor_base &&) noexcept(std::is_nothrow_move_constructible_v<T>) = default;
  delete_copymove_constructor_base& operator=(const delete_copymove_constructor_base &) = default;
  delete_copymove_constructor_base& operator=(delete_copymove_constructor_base &&) noexcept(std::is_nothrow_move_assignable_v<T>) = default;
};

template <typename T>
struct delete_copymove_constructor_base<T, true, false> {
  delete_copymove_constructor_base() = default;

  delete_copymove_constructor_base(const delete_copymove_constructor_base &) = default;
  delete_copymove_constructor_base(delete_copymove_constructor_base &&) noexcept(std::is_nothrow_move_constructible_v<T>) = delete;
  delete_copymove_constructor_base& operator=(const delete_copymove_constructor_base &) = default;
  delete_copymove_constructor_base& operator=(delete_copymove_constructor_base &&) noexcept(std::is_nothrow_move_assignable_v<T>) = default;
};

template <typename T>
struct delete_copymove_constructor_base<T, false, true> {
  delete_copymove_constructor_base() = default;

  delete_copymove_constructor_base(const delete_copymove_constructor_base &) = delete;
  delete_copymove_constructor_base(delete_copymove_constructor_base &&) noexcept(std::is_nothrow_move_constructible_v<T>) = default;
  delete_copymove_constructor_base& operator=(const delete_copymove_constructor_base &) = default;
  delete_copymove_constructor_base& operator=(delete_copymove_constructor_base &&) noexcept(std::is_nothrow_move_assignable_v<T>) = default;
};

template <typename T>
struct delete_copymove_constructor_base<T, false, false> {
  delete_copymove_constructor_base() = default;

  delete_copymove_constructor_base(const delete_copymove_constructor_base &) = delete;
  delete_copymove_constructor_base(delete_copymove_constructor_base &&) noexcept(std::is_nothrow_move_constructible_v<T>) = delete;
  delete_copymove_constructor_base& operator=(const delete_copymove_constructor_base &) = default;
  delete_copymove_constructor_base& operator=(delete_copymove_constructor_base &&) noexcept(std::is_nothrow_move_assignable_v<T>) = default;
};

///////////////////////////////////////////

template <typename T,
          bool = std::is_copy_assignable_v<T>,
          bool = std::is_move_assignable_v<T>>
struct delete_copymove_assignment_base;

template <typename T>
struct delete_copymove_assignment_base<T, true, true> {
  delete_copymove_assignment_base() = default;

  delete_copymove_assignment_base(const delete_copymove_assignment_base &) = default;
  delete_copymove_assignment_base(delete_copymove_assignment_base &&) noexcept(std::is_nothrow_move_constructible_v<T>) = default;
  delete_copymove_assignment_base& operator=(const delete_copymove_assignment_base &) = default;
  delete_copymove_assignment_base& operator=(delete_copymove_assignment_base &&) noexcept(std::is_nothrow_move_assignable_v<T>) = default;
};

template <typename T>
struct delete_copymove_assignment_base<T, true, false> {
  delete_copymove_assignment_base() = default;

  delete_copymove_assignment_base(const delete_copymove_assignment_base &) = default;
  delete_copymove_assignment_base(delete_copymove_assignment_base &&) noexcept(std::is_nothrow_move_constructible_v<T>) = default;
  delete_copymove_assignment_base& operator=(const delete_copymove_assignment_base &) = default;
  delete_copymove_assignment_base& operator=(delete_copymove_assignment_base &&) noexcept(std::is_nothrow_move_assignable_v<T>) = delete;
};

template <typename T>
struct delete_copymove_assignment_base<T, false, true> {
  delete_copymove_assignment_base() = default;

  delete_copymove_assignment_base(const delete_copymove_assignment_base &) = default;
  delete_copymove_assignment_base(delete_copymove_assignment_base &&) noexcept(std::is_nothrow_move_constructible_v<T>) = default;
  delete_copymove_assignment_base& operator=(const delete_copymove_assignment_base &) = delete;
  delete_copymove_assignment_base& operator=(delete_copymove_assignment_base &&) noexcept(std::is_nothrow_move_assignable_v<T>) = default;
};

template <typename T>
struct delete_copymove_assignment_base<T, false, false> {
  delete_copymove_assignment_base() = default;

  delete_copymove_assignment_base(const delete_copymove_assignment_base &) = default;
  delete_copymove_assignment_base(delete_copymove_assignment_base &&) noexcept(std::is_nothrow_move_constructible_v<T>) = default;
  delete_copymove_assignment_base& operator=(const delete_copymove_assignment_base &) = delete;
  delete_copymove_assignment_base& operator=(delete_copymove_assignment_base &&) noexcept(std::is_nothrow_move_assignable_v<T>) = delete;
};

///////////////////////////////////////////
template<typename T>
struct optional : private optional_storage_base<T>
                  , private delete_copymove_constructor_base<T>
                  , private delete_copymove_assignment_base<T>
                  {

  optional() noexcept = default;
  optional(const optional&) noexcept = default;
  optional(optional&&) noexcept = default;
  // FORWARDING_CONSTRUCTORS(optional, optional_storage_base<T>, optional_storage_base<T>);
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
  char* mB;
  constructor_Complex_Nothrow(int aA, char* aB) noexcept : mA(aA), mB(aB) {}
};

struct constructor_Complex_Throw {
  int mA;
  char* mB;
  constructor_Complex_Throw(int aA, char* aB) noexcept(false) : mA(aA), mB(aB) {}
};

struct constructor_Explicit_Single {
  int m;
  explicit constructor_Explicit_Single(int a) : m(a) {}
};

struct constructor_Explicit_Double {
  int mA;
  int mB;
  explicit constructor_Explicit_Double(int aA, int aB) : mA(aA), mB(aB) {}
};

struct constructor_Explicit_Double_Diff {
  int mA;
  char* mB;
  explicit constructor_Explicit_Double_Diff(int aA, char* aB) : mA(aA), mB(aB) {}
};

struct constructor_NoExplicit_Single {
  int m;
  constructor_NoExplicit_Single(int a) : m(a) {}
};

struct constructor_NoExplicit_Double {
  int mA;
  int mB;
  constructor_NoExplicit_Double(int aA, int aB) : mA(aA), mB(aB) {}
};

struct constructor_NoExplicit_Double_Diff {
  int mA;
  char* mB;
  constructor_NoExplicit_Double_Diff(int aA, char* aB) : mA(aA), mB(aB) {}
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
  ~destructor_None() = delete;
};

///////////////////////////////////////////

struct copyConstruct_Trivial_NoThrow {
  int m;
};

struct copyConstruct_NoTrivial_Nothrow {
  int m;
  copyConstruct_NoTrivial_Nothrow() noexcept = default;
  copyConstruct_NoTrivial_Nothrow(const copyConstruct_NoTrivial_Nothrow & aRhs) noexcept {
    m = aRhs.m;
  }
  copyConstruct_NoTrivial_Nothrow(copyConstruct_NoTrivial_Nothrow &&) noexcept = default;
};

struct copyConstruct_NoTrivial_Throw {
  int m;
  copyConstruct_NoTrivial_Throw() noexcept = default;
  copyConstruct_NoTrivial_Throw(const copyConstruct_NoTrivial_Throw & aRhs) noexcept(false) {
    m = aRhs.m;
  }
  copyConstruct_NoTrivial_Throw(copyConstruct_NoTrivial_Throw &&) noexcept = default;
};

struct copyConstruct_None {
  int m;
  copyConstruct_None() noexcept = default;
  copyConstruct_None(const copyConstruct_None &) = delete;
  copyConstruct_None(copyConstruct_None &&) noexcept = default;
};

///////////////////////////////////////////

struct moveConstruct_Trivial_NoThrow {
  int m;
};

struct moveConstruct_NoTrivial_Nothrow {
  int m;
  moveConstruct_NoTrivial_Nothrow() noexcept = default;
  moveConstruct_NoTrivial_Nothrow(moveConstruct_NoTrivial_Nothrow && aRhs) noexcept {
    m = aRhs.m;
  }
  moveConstruct_NoTrivial_Nothrow(const moveConstruct_NoTrivial_Nothrow &) noexcept = default;
};

struct moveConstruct_NoTrivial_Throw {
  int m;
  moveConstruct_NoTrivial_Throw() noexcept = default;
  moveConstruct_NoTrivial_Throw(moveConstruct_NoTrivial_Throw && aRhs) noexcept(false) {
    m = aRhs.m;
  }
  moveConstruct_NoTrivial_Throw(const moveConstruct_NoTrivial_Throw &) noexcept = default;
};

struct moveConstruct_None {
  int m;
  moveConstruct_None() noexcept = default;
  moveConstruct_None(moveConstruct_None &&) = delete;
  moveConstruct_None(const moveConstruct_None &) noexcept = default;
};



///////////////////////////////////////////

#include <iostream>
#include <string>

#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_RESET   "\x1b[0m"

#define MATCH_CHECK(wrapper, trait, T) "  " << #trait << ": " << trait<T> << ", " << (trait<T> == trait<wrapper<T>>? ANSI_COLOR_GREEN "Match" ANSI_COLOR_RESET : ANSI_COLOR_RED "!!!!!!!!!!!NOMATCH!!!!!!" ANSI_COLOR_RESET) << '\n'

template<typename T>
constexpr bool is_constructible_int_charp_v = std::is_constructible_v<T, int&, char*&>;

template<typename T>
constexpr bool is_convertible_int_v = std::is_convertible_v<int, T>;

template<typename T>
constexpr bool is_convertible_int_charp_v = is_convertible_from_many_v<T, int, char*>;

#define printStatus(wrapper, T)                                                \
  std::cout << std::boolalpha << #T << ", " << #wrapper << '<' << #T << ">\n"  \
            << MATCH_CHECK(wrapper, std::is_constructible_v, T)                \
            << MATCH_CHECK(wrapper, std::is_trivially_constructible_v, T)      \
            << MATCH_CHECK(wrapper, std::is_nothrow_constructible_v, T)        \
            << MATCH_CHECK(wrapper, is_constructible_int_charp_v, T)           \
            << MATCH_CHECK(wrapper, std::is_destructible_v, T)                 \
            << MATCH_CHECK(wrapper, std::is_trivially_destructible_v, T)       \
            << MATCH_CHECK(wrapper, std::is_nothrow_destructible_v, T)         \
            << MATCH_CHECK(wrapper, is_convertible_int_v, T)                   \
            << MATCH_CHECK(wrapper, is_convertible_int_charp_v, T)             \
            << MATCH_CHECK(wrapper, std::is_copy_constructible_v, T)           \
            << MATCH_CHECK(wrapper, std::is_trivially_copy_constructible_v, T) \
            << MATCH_CHECK(wrapper, std::is_nothrow_copy_constructible_v, T)   \
            << MATCH_CHECK(wrapper, std::is_move_constructible_v, T)           \
            << MATCH_CHECK(wrapper, std::is_trivially_move_constructible_v, T) \
            << MATCH_CHECK(wrapper, std::is_nothrow_move_constructible_v, T)   \
            << '\n'

template <typename... T>
void rlbox_test_helper_print_type() {
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
  std::cout << __FUNCSIG__ << std::endl;
#else
  std::cout << __PRETTY_FUNCTION__ << std::endl;
#endif
}


int main()
{

  // optional<moveConstruct_None> o;
  // optional_storage_base<moveConstruct_None, true, true>* p1 = &o;
  // delete_copymove_constructor_base<moveConstruct_None, true, false>* p2 = &o;
  // delete_copymove_assignment_base<moveConstruct_None, false, false>* p3 = &o;

  // std::cout << std::boolalpha
  //   << std::is_move_constructible_v<moveConstruct_None> << ' '
  //   << std::is_move_constructible_v<optional<moveConstruct_None>> << ' '
  //   << std::is_move_constructible_v<optional_storage_base<moveConstruct_None, true, true>> << ' '
  //   << std::is_move_constructible_v<delete_copymove_constructor_base<moveConstruct_None, true, false>> << ' '
  //   << std::is_move_constructible_v<delete_copymove_assignment_base<moveConstruct_None, false, false>> << ' '
  //   << '\n';


  constructor_Explicit_Single c4 {1};
  constructor_Explicit_Double c5 {1, 1};
  constructor_Explicit_Double_Diff c6 {1, nullptr};
  constructor_NoExplicit_Single cn4 {1};
  constructor_NoExplicit_Double cn5 {1, 1};
  constructor_NoExplicit_Double_Diff cn6 {1, nullptr};

  optional_storage_base<constructor_Explicit_Single> wc4 {1};
  optional_storage_base<constructor_Explicit_Double> wc5 {1, 1};
  optional_storage_base<constructor_Explicit_Double_Diff> wc6 {1, nullptr};
  optional_storage_base<constructor_NoExplicit_Single> wcn4 {1};
  optional_storage_base<constructor_NoExplicit_Double> wcn5 {1, 1};
  optional_storage_base<constructor_NoExplicit_Double_Diff> wcn6 {1, nullptr};

  // type var = { a,b,c } "initializer" case tested below;

  printStatus(optional_storage_base, constructor_Trivial_NoThrow);
  printStatus(optional_storage_base, constructor_NoTrivial_Nothrow);
  printStatus(optional_storage_base, constructor_NoTrivial_Throw);
  printStatus(optional_storage_base, constructor_Complex_Nothrow);
  printStatus(optional_storage_base, constructor_Complex_Throw);
  printStatus(optional_storage_base, constructor_Explicit_Single);
  printStatus(optional_storage_base, constructor_Explicit_Double);
  printStatus(optional_storage_base, constructor_Explicit_Double_Diff);
  printStatus(optional_storage_base, constructor_NoExplicit_Single);
  printStatus(optional_storage_base, constructor_NoExplicit_Double);
  printStatus(optional_storage_base, constructor_NoExplicit_Double_Diff);
  printStatus(optional_storage_base, constructor_None);
  printStatus(optional_storage_base, copyConstruct_Trivial_NoThrow);
  printStatus(optional_storage_base, copyConstruct_NoTrivial_Nothrow);
  printStatus(optional_storage_base, copyConstruct_NoTrivial_Throw);
  printStatus(optional_storage_base, copyConstruct_None);
  printStatus(optional_storage_base, moveConstruct_Trivial_NoThrow);
  printStatus(optional_storage_base, moveConstruct_NoTrivial_Nothrow);
  printStatus(optional_storage_base, moveConstruct_NoTrivial_Throw);
  printStatus(optional_storage_base, moveConstruct_None);

  printStatus(optional_storage_base, destructor_Trivial_NoThrow);
  printStatus(optional_storage_base, destructor_NoTrivial_Nothrow);
  printStatus(optional_storage_base, destructor_NoTrivial_Throw);
  printStatus(optional_storage_base, destructor_None);
}
