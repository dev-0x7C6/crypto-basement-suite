#include <chrono>
#include <iostream>
#include <vector>

#include <types.hpp>

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <range/v3/all.hpp>
#include <networking/downloader.hpp>

#include <QCoreApplication>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDateTime>

using namespace ranges;
using namespace std::chrono_literals;

namespace fear_and_greed {

struct info {
    i32 value{};
    std::string classification;
    types::time_point timestamp{};
    types::time_point to_next_update{};
};

auto info_from_json(const QJsonObject &object) noexcept {
    info ret;
    ret.value = object.value("value").toString().toInt();
    ret.classification = object.value("value_classification").toString().toLower().toStdString();
    ret.timestamp = object.value("timestamp").toString().toUInt();
    ret.to_next_update = object.value("time_until_update").toString().toUInt();
    return ret;
}

auto generate(const QJsonObject &object) noexcept {
    std::vector<info> ret;
    const auto array = object.value("data").toArray();
    for (auto &&node : array)
        ret.emplace_back(info_from_json(node.toObject()));

    return ret;
}

} // namespace fear_and_greed

auto main(int argc, char **argv) -> int {
    QCoreApplication application(argc, argv);
    auto console = spdlog::stdout_color_mt("console");

    std::vector<fear_and_greed::info> info_table;

    network::downloader dl;
    dl.download({"https://api.alternative.me/fng/?limit=32"}, [&](const QByteArray &data) {
        info_table = fear_and_greed::generate(QJsonDocument::fromJson(data).object());
        application.quit();
    });

    application.exec();

    spdlog::set_pattern("%v");

    for (auto &&entry : info_table) {
        auto date = QDateTime::fromSecsSinceEpoch(entry.timestamp);
        auto date_str = date.toString("yyyy.MM.dd").toStdString();
        spdlog::info("{}, {}: {}", date_str, entry.classification, entry.value);
    }

    return 0;
}
