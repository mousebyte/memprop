#ifndef MB_SIGNAL_IX_HPP
#define MB_SIGNAL_IX_HPP
#include <sigslot/signal.hpp>
#include <optional>

namespace mousebyte {
    namespace detail {
        template <typename Sig, typename... Args>
        concept ConnectCallable =
            requires(Sig sig, Args && ... args)
            {
                {
                sig.connect(std::forward<Args>(args)...)
                }
            -> std::same_as<sigslot::connection>;
            };
        template <typename Sig, typename... Args>
        concept ConnectExtendedCallable =
            requires(Sig sig, Args && ... args)
            {
                {
                sig.connect_extended(std::forward<Args>(args)...)
                }
            -> std::same_as<sigslot::connection>;
            };

        template <typename Sig, typename... Args>
        concept DisconnectCallable =
            requires(Sig sig, Args && ... args)
            {
                {
                sig.disconnect(std::forward<Args>(args)...)
                }
            -> std::same_as<size_t>;
            };

        }


    template <template<typename...> typename Sig, typename Owner, typename... Args>
    class signal_interface final {
            using signal_type = Sig<Args...>;
            std::optional<signal_type> _sigStorage;
            signal_type* _sig;
            friend Owner;

            template <typename... U>
            void operator()(U&& ... args)
                {
                (*_sig)(std::forward<U>(args)...);
                }

            size_t slot_count() noexcept
                {
                return _sig->slot_count();
                }

            void block() noexcept
                {
                _sig->block();
                }

            void unblock() noexcept
                {
                _sig->unblock();
                }

            bool blocked() const noexcept
                {
                return _sig->blocked();
                }

            signal_interface(signal_interface&&) /* not noexcept */ = default;
            signal_interface& operator=(signal_interface&&) /* not noexcept */ = default;
            ~signal_interface() = default;

        public:
            signal_interface()
                : _sigStorage(std::in_place)
                , _sig(std::addressof(*_sigStorage))
                {
                }

            explicit signal_interface(signal_type* sig)
                : _sigStorage(std::nullopt)
                , _sig(sig)
                {
                }

            signal_interface(signal_interface const&) = delete;
            signal_interface& operator=(signal_interface const&) = delete;

            template <typename... Ts>
            requires detail::ConnectCallable<signal_type, Ts...>
            sigslot::connection connect(Ts&& ... args)
                {
                return _sig->connect(std::forward<Ts>(args)...);
                }

            template <typename... Ts>
            requires detail::ConnectExtendedCallable<signal_type, Ts...>
            sigslot::connection connect_extended(Ts&& ... args)
                {
                return _sig->connect_extended(std::forward<Ts>(args)...);
                }

            template <typename... Ts>
            requires detail::ConnectCallable<signal_type, Ts...>
            sigslot::scoped_connection connect_scoped(Ts&& ... args)
                {
                return _sig->connect(std::forward<Ts>(args)...);
                }

            template <typename... Ts>
            requires detail::DisconnectCallable<signal_type, Ts...>
            size_t disconnect(Ts&& ... args)
                {
                return _sig->disconnect(std::forward<Ts>(args)...);
                }

            void disconnect_all()
                {
                _sig->disconnect_all();
                }
        };

    template<typename Owner, typename... Args>
    using signal_ix = signal_interface<sigslot::signal, Owner, Args...>;
    }

#endif
