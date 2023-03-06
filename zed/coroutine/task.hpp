#ifndef ZED_COROUTINE_TASK_HPP_
#define ZED_COROUTINE_TASK_HPP_

#include <cassert>
#include <coroutine>
#include <exception>
#include <variant>

#include <iostream>

namespace zed {

namespace coroutine {

    template <typename T>
    class Task;

    namespace detail {

        struct TaskPromiseBase {
            std::coroutine_handle<> m_parent_coroutine {std::noop_coroutine()};

            struct TaskFinalAwaiter {

                constexpr bool await_ready() const noexcept { return false; }

                template <typename PromiseType>
                std::coroutine_handle<>
                await_suspend(std::coroutine_handle<PromiseType> who_call_me) noexcept
                {
                    // transfer running rights to it's parent coroutine
                    return who_call_me.promise().m_parent_coroutine;
                }

                // This function will not never be executed
                constexpr void await_resume() const noexcept { }
            };

            TaskPromiseBase() noexcept = default;

            constexpr std::suspend_always initial_suspend() const noexcept { return {}; }

            constexpr TaskFinalAwaiter final_suspend() const noexcept { return {}; }
        };

        template <typename T>
        class TaskPromise final : public TaskPromiseBase {
        public:
            TaskPromise() noexcept { }

            ~TaskPromise() noexcept { }

            Task<T> get_return_object() noexcept;

            void unhandled_exception() noexcept
            {
                m_value.template emplace<std::exception_ptr>(std::current_exception());
            }

            template <typename V>
                requires std::is_convertible_v<V&&, T>
            void return_value(V&& value) noexcept(std::is_nothrow_constructible_v<T, V&&>)
            {
                m_value.template emplace<T>(std::forward<V>(value));
            }

            T& result() &
            {

                if (std::holds_alternative<std::exception_ptr>(m_value)) [[unlikely]] {
                    std::rethrow_exception(std::get<std::exception_ptr>(m_value));
                }
                // TODO delete this
                std::cout << std::get<T>(m_value) << std::endl;

                assert(std::holds_alternative<T>(m_value));
                return std::get<T>(m_value);
            }

            T&& result() &&
            {
                if (std::holds_alternative<std::exception_ptr>(m_value)) [[unlikely]] {
                    std::rethrow_exception(std::get<std::exception_ptr>(m_value));
                }
                assert(std::holds_alternative<T>(m_value));
                return std::move(std::get<T>(m_value));
            }

        private:
            // The function of std::monostate is to let std::variant can default construct itself
            std::variant<std::monostate, T, std::exception_ptr> m_value;
        };

        template <>
        class TaskPromise<void> final : public TaskPromiseBase {
        public:
            Task<void> get_return_object() noexcept;

            constexpr void return_void() const noexcept {};

            void unhandled_exception() noexcept { m_exception = std::current_exception(); }

            void result() const
            {
                if (m_exception != nullptr) [[unlikely]] {
                    std::rethrow_exception(m_exception);
                }
            }

        private:
            std::exception_ptr m_exception {};
        };

        // template <typename T>
        // class TaskPromise<T&> final : public TaskPromiseBase {
        // public:
        //     TaskPromise() noexcept = default;

        //     Task<T&> get_return_object() noexcept
        //     {
        //         Task<T&> {std::coroutine_handle<TaskPromise>::from_promise(*this)};
        //     }

        //     void unhaneled_exception() noexcept { m_exception = std::current_exception(); }

        //     void return_value(T& value) noexcept { m_value = std::addressof(value); }

        //     T& result()
        //     {
        //         if (m_exception) {
        //             std::rethrow_exception(m_exception);
        //         }
        //     }

        // private:
        //     T*                 m_value {nullptr};
        //     std::exception_ptr m_exception {};
        // };

    } // namespace detail

    template <typename T = void>
    class [[nodiscard]] Task {
    public:
        using promise_type = detail::TaskPromise<T>;
        using ValueType = T;

    private:
        struct AwaiterBase {

            std::coroutine_handle<promise_type> m_handle;

            explicit AwaiterBase(std::coroutine_handle<promise_type> handle) noexcept
                : m_handle(handle)
            {
            }

            [[nodiscard]] inline bool await_ready() const noexcept
            {
                return !m_handle || m_handle.done();
            }

            std::coroutine_handle<> await_suspend(std::coroutine_handle<> who_call_me) noexcept
            {
                m_handle.promise().m_parent_coroutine = who_call_me;
                return m_handle;
            }
        };

    public:
        Task() noexcept = default;

        explicit Task(std::coroutine_handle<promise_type> handle) : m_handle(handle) { }

        Task(const Task&) = delete;
        Task& operator=(const Task&) = delete;

        Task(Task&& other) noexcept : m_handle(other.m_handle) { other.m_handle = nullptr; }

        Task& operator=(Task&& other) noexcept
        {
            if (std::addressof(other) != this) [[likely]] {
                if (m_handle) {
                    m_handle.destroy();
                }
                m_handle = other.m_handle;
                other.m_handle = nullptr;
            }
            return *this;
        }

        ~Task()
        {
            if (m_handle) {
                m_handle.destroy();
            }
        }

        auto operator co_await() const& noexcept
        {
            struct Awaiter final : public AwaiterBase {
                using AwaiterBase::AwaiterBase;

                decltype(auto) await_resume()
                {
                    assert(this->m_handle && "broken_promise");
                    return this->m_handle.promise().result();
                }
            };

            return Awaiter {m_handle};
        }

        auto operator co_await() const&& noexcept
        {
            struct Awaiter final : public AwaiterBase {
                using AwaiterBase::AwaiterBase;

                decltype(auto) await_resume()
                {
                    assert(this->m_handle && "broken_promise");
                    return this->m_handle.promise().result();
                }
            };

            return Awaiter {m_handle};
        }

        [[nodiscard]] inline bool isReady() const noexcept { return !m_handle || m_handle.done(); }

        // If you don't care the result, do it
        [[nodiscard]] auto whenReady() const noexcept
        {
            struct Awaiter final : public AwaiterBase {
                using AwaiterBase::AwaiterBase;

                constexpr void await_resume() const noexcept {};
            };

            return Awaiter {m_handle};
        }

        void detach() noexcept { m_handle = nullptr; }

        [[nodiscard]] auto getHandle() const noexcept { return m_handle; }

        friend void swap(Task& lhs, Task& rhs) noexcept { std::swap(lhs.m_handle, rhs.m_handle); }

    private:
        std::coroutine_handle<promise_type> m_handle;
    };

    namespace detail {

        template <typename T>
        inline Task<T> TaskPromise<T>::get_return_object() noexcept
        {
            return Task<T> {std::coroutine_handle<TaskPromise>::from_promise(*this)};
        }

        inline Task<void> TaskPromise<void>::get_return_object() noexcept
        {
            return Task<void> {std::coroutine_handle<TaskPromise>::from_promise(*this)};
        }

    } // namespace detail

} // namespace coroutine

} // namespace zed

#endif // ZED_COROUTINE_TASK
