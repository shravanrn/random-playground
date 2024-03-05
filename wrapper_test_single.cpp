#include <optional>
#include <type_traits>

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

template <typename T,
          bool = std::is_trivially_constructible_v<T>,
          bool = std::is_destructible_v<T>,
          bool = std::is_trivially_destructible_v<T>,
          bool = std::is_copy_constructible_v<T>,
          bool = std::is_trivially_copy_constructible_v<T>,
          bool = std::is_move_constructible_v<T>,
          bool = std::is_trivially_move_constructible_v<T>,
          bool = std::is_copy_assignable_v<T>,
          bool = std::is_trivially_copy_assignable_v<T>,
          bool = std::is_move_assignable_v<T>,
          bool = std::is_trivially_move_assignable_v<T>>
struct optional_storage_base;


#define WRAPPER_CLASS_MACRO_0(BODY, TRIVIALLY_CONSTRUCTIBLE, DESTRUCTIBLE, TRIVIALLY_DESTRUCTIBLE, COPY_CONSTRUCTIBLE, TRIVIALLY_COPY_CONSTRUCTIBLE, MOVE_CONSTRUCTIBLE, TRIVIALLY_MOVE_CONSTRUCTIBLE, COPY_ASSIGNABLE, TRIVIALLY_COPY_ASSIGNABLE, MOVE_ASSIGNABLE, TRIVIALLY_MOVE_ASSIGNABLE) \
template <typename T>                                                                                                                                                                                                                                                                        \
struct optional_storage_base<T, TRIVIALLY_CONSTRUCTIBLE, DESTRUCTIBLE, TRIVIALLY_DESTRUCTIBLE, COPY_CONSTRUCTIBLE, TRIVIALLY_COPY_CONSTRUCTIBLE, MOVE_CONSTRUCTIBLE, TRIVIALLY_MOVE_CONSTRUCTIBLE, COPY_ASSIGNABLE, TRIVIALLY_COPY_ASSIGNABLE, MOVE_ASSIGNABLE, TRIVIALLY_MOVE_ASSIGNABLE> { \
  T m_value;                                                                                                                                                                                                                                                                                 \
                                                                                                                                                                                                                                                                                             \
  template <class... U, std::enable_if_t<std::is_constructible_v<T, U...> && is_convertible_from_many_v<T, U...>>* = nullptr>                                                                                                                                                                \
  optional_storage_base(U&&... u) noexcept(std::is_nothrow_constructible_v<T, U...>): m_value(std::forward<U>(u)...) {}                                                                                                                                                                      \
                                                                                                                                                                                                                                                                                             \
  template <class... U, std::enable_if_t<std::is_constructible_v<T, U...> && !is_convertible_from_many_v<T, U...>>* = nullptr>                                                                                                                                                               \
  explicit optional_storage_base(U&&... u) noexcept(std::is_nothrow_constructible_v<T, U...>): m_value(std::forward<U>(u)...) {}                                                                                                                                                             \
                                                                                                                                                                                                                                                                                             \
  BODY                                                                                                                                                                                                                                                                                       \
                                                                                                                                                                                                                                                                                             \
  inline constexpr auto& operator*() & noexcept(noexcept(*m_value)) { return *m_value; }                                                                                                                                                                                                     \
  inline constexpr const auto& operator*() const & noexcept(noexcept(*m_value)) { return *m_value; }                                                                                                                                                                                         \
  inline constexpr auto&& operator*() && noexcept(noexcept(*std::move(m_value))) { return *std::move(m_value); }                                                                                                                                                                             \
  inline constexpr const auto&& operator*() const && noexcept(noexcept(*std::move(m_value))) { return *std::move(m_value); }                                                                                                                                                                 \
                                                                                                                                                                                                                                                                                             \
  inline constexpr auto& operator&() & noexcept(noexcept(&m_value)) { return &m_value; }                                                                                                                                                                                                     \
  inline constexpr const auto& operator&() const & noexcept(noexcept(&m_value)) { return &m_value; }                                                                                                                                                                                         \
  inline constexpr auto&& operator&() && noexcept(noexcept(&std::move(m_value))) { return &std::move(m_value); }                                                                                                                                                                             \
  inline constexpr const auto&& operator&() const && noexcept(noexcept(&std::move(m_value))) { return &std::move(m_value); }                                                                                                                                                                 \
};

/* Compares two optional_storage_base objects */
template <class T, class U>
inline constexpr auto operator==(const optional_storage_base<T> &lhs, const optional_storage_base<U> &rhs) noexcept(noexcept(lhs.m_value == rhs.m_value)) { return lhs.m_value == rhs.m_value; }

template <class T, class U>
inline constexpr auto operator!=(const optional_storage_base<T> &lhs, const optional_storage_base<U> &rhs) noexcept(noexcept(lhs.m_value != rhs.m_value)) { return lhs.m_value != rhs.m_value; }

template <class T, class U>
inline constexpr auto operator<(const optional_storage_base<T> &lhs, const optional_storage_base<U> &rhs) noexcept(noexcept(lhs.m_value < rhs.m_value)) { return lhs.m_value < rhs.m_value; }

template <class T, class U>
inline constexpr auto operator>(const optional_storage_base<T> &lhs, const optional_storage_base<U> &rhs) noexcept(noexcept(lhs.m_value > rhs.m_value)) { return lhs.m_value > rhs.m_value; }

template <class T, class U>
inline constexpr auto operator<=(const optional_storage_base<T> &lhs, const optional_storage_base<U> &rhs) noexcept(noexcept(lhs.m_value <= rhs.m_value)) { return lhs.m_value <= rhs.m_value; }

template <class T, class U>
inline constexpr auto operator>=(const optional_storage_base<T> &lhs, const optional_storage_base<U> &rhs) noexcept(noexcept(lhs.m_value >= rhs.m_value)) { return lhs.m_value >= rhs.m_value; }

/* Compares the optional_storage_base with a value */
template <class T, class U>
inline constexpr auto operator==(const optional_storage_base<T> &lhs, const U &rhs) noexcept(noexcept(lhs.m_value == rhs)) { return lhs.m_value == rhs; }

template <class T, class U>
inline constexpr auto operator==(const U &lhs, const optional_storage_base<T> &rhs) noexcept(noexcept(lhs == rhs.m_value)) { return lhs == rhs.m_value; }

template <class T, class U>
inline constexpr auto operator!=(const optional_storage_base<T> &lhs, const U &rhs) noexcept(noexcept(lhs.m_value != rhs)) { return lhs.m_value != rhs; }

template <class T, class U>
inline constexpr auto operator!=(const U &lhs, const optional_storage_base<T> &rhs) noexcept(noexcept(lhs != rhs.m_value)) { return lhs != rhs.m_value; }

template <class T, class U>
inline constexpr auto operator<(const optional_storage_base<T> &lhs, const U &rhs) noexcept(noexcept(lhs.m_value < rhs)) { return lhs.m_value < rhs; }

template <class T, class U>
inline constexpr auto operator<(const U &lhs, const optional_storage_base<T> &rhs) noexcept(noexcept(lhs < rhs.m_value)) { return lhs < rhs.m_value; }

template <class T, class U>
inline constexpr auto operator<=(const optional_storage_base<T> &lhs, const U &rhs) noexcept(noexcept(lhs.m_value <= rhs)) { return lhs.m_value <= rhs; }

template <class T, class U>
inline constexpr auto operator<=(const U &lhs, const optional_storage_base<T> &rhs) noexcept(noexcept(lhs <= rhs.m_value)) { return lhs <= rhs.m_value; }

template <class T, class U>
inline constexpr auto operator>(const optional_storage_base<T> &lhs, const U &rhs) noexcept(noexcept(lhs.m_value > rhs)) { return lhs.m_value > rhs; }

template <class T, class U>
inline constexpr auto operator>(const U &lhs, const optional_storage_base<T> &rhs) noexcept(noexcept(lhs > rhs.m_value)) { return lhs > rhs.m_value; }

template <class T, class U>
inline constexpr auto operator>=(const optional_storage_base<T> &lhs, const U &rhs) noexcept(noexcept(lhs.m_value >= rhs)) { return lhs.m_value >= rhs; }

template <class T, class U>
inline constexpr auto operator>=(const U &lhs, const optional_storage_base<T> &rhs) noexcept(noexcept(lhs >= rhs.m_value)) { return lhs >= rhs.m_value; }

// Iterate over TRIVIALLY_CONSTRUCTIBLE
#define TRIVIAL_CONSTRUCTOR inline optional_storage_base() = default;
#define NONTRIVIAL_CONSTRUCTOR

#define WRAPPER_CLASS_MACRO_1(BODY, DESTRUCTIBLE, TRIVIALLY_DESTRUCTIBLE, COPY_CONSTRUCTIBLE, TRIVIALLY_COPY_CONSTRUCTIBLE, MOVE_CONSTRUCTIBLE, TRIVIALLY_MOVE_CONSTRUCTIBLE, COPY_ASSIGNABLE, TRIVIALLY_COPY_ASSIGNABLE, MOVE_ASSIGNABLE, TRIVIALLY_MOVE_ASSIGNABLE)                     \
WRAPPER_CLASS_MACRO_0(BODY TRIVIAL_CONSTRUCTOR, true, DESTRUCTIBLE, TRIVIALLY_DESTRUCTIBLE, COPY_CONSTRUCTIBLE, TRIVIALLY_COPY_CONSTRUCTIBLE, MOVE_CONSTRUCTIBLE, TRIVIALLY_MOVE_CONSTRUCTIBLE, COPY_ASSIGNABLE, TRIVIALLY_COPY_ASSIGNABLE, MOVE_ASSIGNABLE, TRIVIALLY_MOVE_ASSIGNABLE)     \
WRAPPER_CLASS_MACRO_0(BODY NONTRIVIAL_CONSTRUCTOR, false, DESTRUCTIBLE, TRIVIALLY_DESTRUCTIBLE, COPY_CONSTRUCTIBLE, TRIVIALLY_COPY_CONSTRUCTIBLE, MOVE_CONSTRUCTIBLE, TRIVIALLY_MOVE_CONSTRUCTIBLE, COPY_ASSIGNABLE, TRIVIALLY_COPY_ASSIGNABLE, MOVE_ASSIGNABLE, TRIVIALLY_MOVE_ASSIGNABLE) \

// Iterate over DESTRUCTIBLE, TRIVIALLY_DESTRUCTIBLE
#define TRIVIAL_DESTRUCTOR inline ~optional_storage_base() = default;
#define NONTRIVIAL_DESTRUCTOR inline ~optional_storage_base() noexcept(std::is_nothrow_destructible_v<T>) { m_value.~T(); }
#define NONE_DESTRUCTOR inline ~optional_storage_base() = delete;

#define WRAPPER_CLASS_MACRO_2(BODY, COPY_CONSTRUCTIBLE, TRIVIALLY_COPY_CONSTRUCTIBLE, MOVE_CONSTRUCTIBLE, TRIVIALLY_MOVE_CONSTRUCTIBLE, COPY_ASSIGNABLE, TRIVIALLY_COPY_ASSIGNABLE, MOVE_ASSIGNABLE, TRIVIALLY_MOVE_ASSIGNABLE)                            \
WRAPPER_CLASS_MACRO_1(BODY TRIVIAL_DESTRUCTOR, true, true, COPY_CONSTRUCTIBLE, TRIVIALLY_COPY_CONSTRUCTIBLE, MOVE_CONSTRUCTIBLE, TRIVIALLY_MOVE_CONSTRUCTIBLE, COPY_ASSIGNABLE, TRIVIALLY_COPY_ASSIGNABLE, MOVE_ASSIGNABLE, TRIVIALLY_MOVE_ASSIGNABLE)     \
WRAPPER_CLASS_MACRO_1(BODY NONTRIVIAL_DESTRUCTOR, true, false, COPY_CONSTRUCTIBLE, TRIVIALLY_COPY_CONSTRUCTIBLE, MOVE_CONSTRUCTIBLE, TRIVIALLY_MOVE_CONSTRUCTIBLE, COPY_ASSIGNABLE, TRIVIALLY_COPY_ASSIGNABLE, MOVE_ASSIGNABLE, TRIVIALLY_MOVE_ASSIGNABLE) \
WRAPPER_CLASS_MACRO_1(BODY NONE_DESTRUCTOR, false, false, COPY_CONSTRUCTIBLE, TRIVIALLY_COPY_CONSTRUCTIBLE, MOVE_CONSTRUCTIBLE, TRIVIALLY_MOVE_CONSTRUCTIBLE, COPY_ASSIGNABLE, TRIVIALLY_COPY_ASSIGNABLE, MOVE_ASSIGNABLE, TRIVIALLY_MOVE_ASSIGNABLE)      \

// Iterate over COPY_CONSTRUCTIBLE, TRIVIALLY_COPY_CONSTRUCTIBLE
#define TRIVIAL_COPYCONSTRUCTOR inline optional_storage_base(const optional_storage_base &) = default;
#define NONTRIVIAL_COPYCONSTRUCTOR inline optional_storage_base(const optional_storage_base & aArg) noexcept(std::is_nothrow_copy_constructible_v<T>) : m_value(aArg.m_value) { }
#define NONE_COPYCONSTRUCTOR inline optional_storage_base(const optional_storage_base &) = delete;

#define WRAPPER_CLASS_MACRO_3(BODY, MOVE_CONSTRUCTIBLE, TRIVIALLY_MOVE_CONSTRUCTIBLE, COPY_ASSIGNABLE, TRIVIALLY_COPY_ASSIGNABLE, MOVE_ASSIGNABLE, TRIVIALLY_MOVE_ASSIGNABLE)                                 \
WRAPPER_CLASS_MACRO_2(BODY TRIVIAL_COPYCONSTRUCTOR, true, true, MOVE_CONSTRUCTIBLE, TRIVIALLY_MOVE_CONSTRUCTIBLE, COPY_ASSIGNABLE, TRIVIALLY_COPY_ASSIGNABLE, MOVE_ASSIGNABLE, TRIVIALLY_MOVE_ASSIGNABLE)     \
WRAPPER_CLASS_MACRO_2(BODY NONTRIVIAL_COPYCONSTRUCTOR, true, false, MOVE_CONSTRUCTIBLE, TRIVIALLY_MOVE_CONSTRUCTIBLE, COPY_ASSIGNABLE, TRIVIALLY_COPY_ASSIGNABLE, MOVE_ASSIGNABLE, TRIVIALLY_MOVE_ASSIGNABLE) \
WRAPPER_CLASS_MACRO_2(BODY NONE_COPYCONSTRUCTOR, false, false, MOVE_CONSTRUCTIBLE, TRIVIALLY_MOVE_CONSTRUCTIBLE, COPY_ASSIGNABLE, TRIVIALLY_COPY_ASSIGNABLE, MOVE_ASSIGNABLE, TRIVIALLY_MOVE_ASSIGNABLE)      \

// Iterate over MOVE_CONSTRUCTIBLE, TRIVIALLY_MOVE_CONSTRUCTIBLE
#define TRIVIAL_MOVECONSTRUCTOR inline optional_storage_base(optional_storage_base &&) = default;
#define NONTRIVIAL_MOVECONSTRUCTOR inline optional_storage_base(optional_storage_base && aArg) noexcept(std::is_nothrow_move_constructible_v<T>) : m_value(std::move(aArg.m_value)) { }
#define NONE_MOVECONSTRUCTOR inline optional_storage_base(optional_storage_base &&) = delete;

#define WRAPPER_CLASS_MACRO_4(BODY, COPY_ASSIGNABLE, TRIVIALLY_COPY_ASSIGNABLE, MOVE_ASSIGNABLE, TRIVIALLY_MOVE_ASSIGNABLE)                                 \
WRAPPER_CLASS_MACRO_3(BODY TRIVIAL_MOVECONSTRUCTOR, true, true, COPY_ASSIGNABLE, TRIVIALLY_COPY_ASSIGNABLE, MOVE_ASSIGNABLE, TRIVIALLY_MOVE_ASSIGNABLE)     \
WRAPPER_CLASS_MACRO_3(BODY NONTRIVIAL_MOVECONSTRUCTOR, true, false, COPY_ASSIGNABLE, TRIVIALLY_COPY_ASSIGNABLE, MOVE_ASSIGNABLE, TRIVIALLY_MOVE_ASSIGNABLE) \
WRAPPER_CLASS_MACRO_3(BODY NONE_MOVECONSTRUCTOR, false, false, COPY_ASSIGNABLE, TRIVIALLY_COPY_ASSIGNABLE, MOVE_ASSIGNABLE, TRIVIALLY_MOVE_ASSIGNABLE)      \

// Iterate over COPY_ASSIGNABLE, TRIVIALLY_COPY_ASSIGNABLE
#define TRIVIAL_COPYASSIGNABLE inline optional_storage_base& operator=(const optional_storage_base &) = default;
#define NONTRIVIAL_COPYASSIGNABLE inline optional_storage_base& operator=(const optional_storage_base & aArg) noexcept(std::is_nothrow_copy_assignable_v<T>) { m_value = aArg.m_value; return *this; }
#define NONE_COPYASSIGNABLE inline optional_storage_base& operator=(const optional_storage_base &) = delete;

#define WRAPPER_CLASS_MACRO_5(BODY, MOVE_ASSIGNABLE, TRIVIALLY_MOVE_ASSIGNABLE)                                 \
WRAPPER_CLASS_MACRO_4(BODY TRIVIAL_COPYASSIGNABLE, true, true, MOVE_ASSIGNABLE, TRIVIALLY_MOVE_ASSIGNABLE)     \
WRAPPER_CLASS_MACRO_4(BODY NONTRIVIAL_COPYASSIGNABLE, true, false, MOVE_ASSIGNABLE, TRIVIALLY_MOVE_ASSIGNABLE) \
WRAPPER_CLASS_MACRO_4(BODY NONE_COPYASSIGNABLE, false, false, MOVE_ASSIGNABLE, TRIVIALLY_MOVE_ASSIGNABLE)      \

// Iterate over MOVE_ASSIGNABLE, TRIVIALLY_MOVE_ASSIGNABLE
#define TRIVIAL_MOVEASSIGNABLE inline optional_storage_base& operator=(optional_storage_base &&) = default;
#define NONTRIVIAL_MOVEASSIGNABLE inline optional_storage_base& operator=(optional_storage_base && aArg) noexcept(std::is_nothrow_move_assignable_v<T>) { m_value = std::move(aArg.m_value); return *this; }
#define NONE_MOVEASSIGNABLE inline optional_storage_base& operator=(optional_storage_base &&) = delete;

#define WRAPPER_CLASS_MACRO_6()                                \
WRAPPER_CLASS_MACRO_5( TRIVIAL_MOVEASSIGNABLE, true, true)     \
WRAPPER_CLASS_MACRO_5( NONTRIVIAL_MOVEASSIGNABLE, true, false) \
WRAPPER_CLASS_MACRO_5( NONE_MOVEASSIGNABLE, false, false)      \

#define WRAPPER_CLASS_MACRO WRAPPER_CLASS_MACRO_6

WRAPPER_CLASS_MACRO()


////////////////////////////////////////////

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

struct copyAssign_Trivial_NoThrow {
  int m;
};
struct copyAssign_NoTrivial_Nothrow {
  int m;
  copyAssign_NoTrivial_Nothrow& operator=(const copyAssign_NoTrivial_Nothrow & aArg) noexcept { m = aArg.m; return *this; }
};

struct copyAssign_NoTrivial_Throw {
  int m;
  copyAssign_NoTrivial_Throw& operator=(const copyAssign_NoTrivial_Throw & aArg) noexcept(false) { m = aArg.m; return *this; }
};

struct copyAssign_None {
  int m;
  copyAssign_None& operator=(const copyAssign_None &) = delete;
};

///////////////////////////////////////////

struct moveAssign_Trivial_NoThrow {
  int m;
};
struct moveAssign_NoTrivial_Nothrow {
  int m;
  moveAssign_NoTrivial_Nothrow& operator=(moveAssign_NoTrivial_Nothrow && aArg) noexcept { m = std::move(aArg.m); return *this; }
};

struct moveAssign_NoTrivial_Throw {
  int m;
  moveAssign_NoTrivial_Throw& operator=(moveAssign_NoTrivial_Throw && aArg) noexcept(false) { m = std::move(aArg.m); return *this; }
};

struct moveAssign_None {
  int m;
  moveAssign_None& operator=(moveAssign_None &&) = delete;
};

///////////////////////////////////////////

struct operatorDeref {

  int m1; int m2; int m3; int m4;
  int* p1; int* p2; int* p3; int* p4;
  operatorDeref() : m1(1), m2(2), m3(3), m4(4), p1(&m1), p2(&m2), p3(&m3), p4(&m4) {}

  constexpr auto& operator*() & noexcept { return *p1; }
  constexpr const auto& operator*() const & { return *p2; }
  constexpr auto&& operator*() && { return std::move(*p3); }
  constexpr const auto&& operator*() const && { return std::move(*p4); }
};

///////////////////////////////////////////

#include <iostream>
#include <string>

#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_RESET   "\x1b[0m"

#define MATCH_CHECK2(wrapper, trait, T) std::cout << "  " << #trait << ": " << trait<T> << ", " << (trait<T> == trait<wrapper<T>>? ANSI_COLOR_GREEN "Match" ANSI_COLOR_RESET : ANSI_COLOR_RED "!!!!!!!!!!!NOMATCH!!!!!!" ANSI_COLOR_RESET) << '\n'
#define MATCH_CHECK(wrapper, trait, T) if (trait<T> != trait<wrapper<T>>) { MATCH_CHECK2(wrapper, trait, T);  abort(); }

template<typename T>
constexpr bool is_constructible_int_charp_v = std::is_constructible_v<T, int&, char*&>;

template<typename T>
constexpr bool is_convertible_int_v = std::is_convertible_v<int, T>;

template<typename T>
constexpr bool is_convertible_int_charp_v = is_convertible_from_many_v<T, int, char*>;

#define printStatus(wrapper, T)                                                \
  std::cout << std::boolalpha << #T << ", " << #wrapper << '<' << #T << ">\n"; \
  MATCH_CHECK(wrapper, std::is_constructible_v, T);                            \
  MATCH_CHECK(wrapper, std::is_trivially_constructible_v, T);                  \
  MATCH_CHECK(wrapper, std::is_nothrow_constructible_v, T);                    \
  MATCH_CHECK(wrapper, is_constructible_int_charp_v, T);                       \
  MATCH_CHECK(wrapper, std::is_destructible_v, T);                             \
  MATCH_CHECK(wrapper, std::is_trivially_destructible_v, T);                   \
  MATCH_CHECK(wrapper, std::is_nothrow_destructible_v, T);                     \
  MATCH_CHECK(wrapper, is_convertible_int_v, T);                               \
  MATCH_CHECK(wrapper, is_convertible_int_charp_v, T);                         \
  MATCH_CHECK(wrapper, std::is_copy_constructible_v, T);                       \
  MATCH_CHECK(wrapper, std::is_trivially_copy_constructible_v, T);             \
  MATCH_CHECK(wrapper, std::is_nothrow_copy_constructible_v, T);               \
  MATCH_CHECK(wrapper, std::is_move_constructible_v, T);                       \
  MATCH_CHECK(wrapper, std::is_trivially_move_constructible_v, T);             \
  MATCH_CHECK(wrapper, std::is_nothrow_move_constructible_v, T);               \
  MATCH_CHECK(wrapper, std::is_copy_assignable_v, T);                          \
  MATCH_CHECK(wrapper, std::is_trivially_copy_assignable_v, T);                \
  MATCH_CHECK(wrapper, std::is_nothrow_copy_assignable_v, T);                  \
  MATCH_CHECK(wrapper, std::is_move_assignable_v, T);                          \
  MATCH_CHECK(wrapper, std::is_trivially_move_assignable_v, T);                \
  MATCH_CHECK(wrapper, std::is_nothrow_move_assignable_v, T);                  \
  std::cout << '\n'

template <typename... T>
void rlbox_test_helper_print_type() {
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
  std::cout << __FUNCSIG__ << std::endl;
#else
  std::cout << __PRETTY_FUNCTION__ << std::endl;
#endif
}

template<typename T>
class optional_derived : public optional_storage_base<T> {};

#define TEST_ASSERT(...) if (!(__VA_ARGS__)) { std::cout << ANSI_COLOR_RED "!!!!!!!!!!!Test failed!!!!!! " ANSI_COLOR_RESET << __FILE__ ":"  << __LINE__ << "\n"; }

int main()
{
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
  printStatus(optional_storage_base, copyAssign_Trivial_NoThrow);
  printStatus(optional_storage_base, copyAssign_NoTrivial_Nothrow);
  printStatus(optional_storage_base, copyAssign_NoTrivial_Throw);
  printStatus(optional_storage_base, copyAssign_None);
  printStatus(optional_storage_base, moveAssign_Trivial_NoThrow);
  printStatus(optional_storage_base, moveAssign_NoTrivial_Nothrow);
  printStatus(optional_storage_base, moveAssign_NoTrivial_Throw);
  printStatus(optional_storage_base, moveAssign_None);

  printStatus(optional_storage_base, destructor_Trivial_NoThrow);
  printStatus(optional_storage_base, destructor_NoTrivial_Nothrow);
  printStatus(optional_storage_base, destructor_NoTrivial_Throw);
  printStatus(optional_storage_base, destructor_None);

  char* p = nullptr;
  int test = 3;

  optional_storage_base<int> test2 = 3;
  TEST_ASSERT(test2 == 3);

  std::cout << std::boolalpha << noexcept(test == 3) << " " << noexcept(test2 == 3) << "\n";

  {
    operatorDeref o;
    optional_storage_base<operatorDeref> wo;
    TEST_ASSERT(*o == *wo);
    TEST_ASSERT(noexcept(*o) == noexcept(*wo));
  }
  {
    const operatorDeref o;
    const optional_storage_base<operatorDeref> wo;
    TEST_ASSERT(*o == *wo);
    TEST_ASSERT(noexcept(*o) == noexcept(*wo));
  }
  {
    const operatorDeref o;
    optional_storage_base<const operatorDeref> wo;
    TEST_ASSERT(*o == *wo);
    TEST_ASSERT(noexcept(*o) == noexcept(*wo));
  }
  {
    TEST_ASSERT(*operatorDeref() == *optional_storage_base<operatorDeref>());
    TEST_ASSERT(noexcept(*operatorDeref()) == noexcept(*optional_storage_base<operatorDeref>()));
  }
  {
    using const_ty = const operatorDeref;
    TEST_ASSERT(*const_ty() == *optional_storage_base<const_ty>());
    TEST_ASSERT(noexcept(*const_ty()) == noexcept(*optional_storage_base<const_ty>()));
  }

  // {
  //   std::optional<int> a1 = 3;
  //   std::optional<int> a2 = 4;

  //   int i1 = 3;
  //   int i2 = 4;

  //   optional_storage_base<std::optional<int>> f1 = 3;
  //   optional_storage_base<std::optional<int>> f1b = 3;
  //   optional_storage_base<std::optional<int>> f2 = 4;

  //   TEST_ASSERT(f1 == f1b);

  //   TEST_ASSERT(f1 == a1);
  //   TEST_ASSERT(f1 == i1);

  //   // TEST_ASSERT(a1 == f1);
  //   TEST_ASSERT(i1 == f1);
  // }
}
