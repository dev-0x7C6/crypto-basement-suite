#pragma once

#include <algorithm>
#include <meta>
#include <optional>
#include <string>
#include <vector>

namespace midnight::airdrop {

struct thaw {
    enum class estatus {
        unknown,
        confirmed,
        redeemable,
        skipped,
        upcoming,
    };

    double amount{};
    std::optional<estatus> status{};
    std::string thawing_period_start;
};

constexpr auto is_redeemable(const thaw::estatus status) noexcept //
{ return thaw::estatus::redeemable == status; }
constexpr auto is_redeemable(const std::optional<thaw::estatus> &status) noexcept //
{ return thaw::estatus::redeemable == status; }
constexpr auto is_redeemable(const thaw &thaw) noexcept //
{ return is_redeemable(thaw.status); }

constexpr auto is_confirmed(const thaw::estatus status) noexcept //
{ return thaw::estatus::confirmed == status; }
constexpr auto is_confirmed(const std::optional<thaw::estatus> &status) noexcept //
{ return thaw::estatus::confirmed == status; }
constexpr auto is_confirmed(const thaw &thaw) noexcept //
{ return is_confirmed(thaw.status); }

constexpr auto is_skipped(const thaw::estatus status) noexcept //
{ return thaw::estatus::skipped == status; }
constexpr auto is_skipped(const std::optional<thaw::estatus> &status) noexcept //
{ return thaw::estatus::skipped == status; }
constexpr auto is_skipped(const thaw &thaw) noexcept //
{ return is_skipped(thaw.status); }

constexpr auto count_redeemable_thaws(const std::vector<thaw> &thaws) noexcept {
    return std::ranges::count_if(thaws, [](auto &&thaw) {
        return //
            is_confirmed(thaw) //
            || is_redeemable(thaw) //
            || is_skipped(thaw);
    });
}

constexpr auto accumulate_redeemable_thaws(const std::vector<thaw> &thaws) noexcept -> double {
    double result{};
    for (auto &&thaw : thaws)
        if (is_confirmed(thaw) || is_redeemable(thaw))
            result += thaw.amount;

    return result;
}

constexpr auto operator<=>(const thaw &lhs, const thaw &rhs) noexcept {
    const auto lv = is_redeemable(lhs.status) ? lhs.amount : 0;
    const auto rv = is_redeemable(rhs.status) ? rhs.amount : 0;
    return lv <=> rv;
}

using thaws_vec = std::vector<thaw>;

struct address {
    std::string id;
    thaws_vec thaws;
};

constexpr auto operator<=>(const address &lhs, const address &rhs) noexcept {
    return lhs.id <=> rhs.id;
}

using addresses_vec = std::vector<address>;

struct stats {
    double total{};
    double total_redeemable{};
    double total_redeemed{};
    int total_thaw_count{};
    int pending_thaw_count{};
    int redeemable_thaw_count{};
    int redeemed_thaw_count{};
    int accessible_thaw_count{};
};

namespace reflection {
template <typename T>
constexpr T &plus_eq_operator(T &self, const T &other) {
    constexpr static auto ctx = std::meta::access_context::current();
    constexpr static auto members = std::define_static_array(std::meta::nonstatic_data_members_of(^^T, ctx));

    template for (constexpr auto member : members)
        self.[:member:] += other.
             [:member:];

    return self;
}
} // namespace reflection

constexpr auto operator+=(stats &stat, const stats &other) noexcept {
    return reflection::plus_eq_operator(stat, other);
}

constexpr auto extract_thaw(const midnight::airdrop::thaw &thaw) noexcept -> midnight::airdrop::stats {
    midnight::airdrop::stats stats{};
    stats.total += is_skipped(thaw) ? 0 : thaw.amount;
    stats.total_thaw_count++;

    if (is_redeemable(thaw)) {
        stats.total_redeemable += thaw.amount;
        stats.redeemable_thaw_count++;
        stats.accessible_thaw_count++;
    }

    if (is_skipped(thaw)) {
        stats.redeemable_thaw_count++;
        stats.accessible_thaw_count++;
    }

    if (is_confirmed(thaw)) {
        stats.total_redeemed += thaw.amount;
        stats.redeemed_thaw_count++;
        stats.accessible_thaw_count++;
    }

    stats.pending_thaw_count = //
        stats.total_thaw_count //
        - stats.redeemable_thaw_count //
        - stats.redeemed_thaw_count;

    return stats;
}

constexpr auto extract_stats(const midnight::airdrop::addresses_vec &addresses) noexcept -> midnight::airdrop::stats {
    midnight::airdrop::stats stats{};

    for (auto &&addr : addresses)
        for (auto &&thaw : addr.thaws)
            stats += extract_thaw(thaw);

    return stats;
}

} // namespace midnight::airdrop
