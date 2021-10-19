#include <chrono>
#include <iostream>

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <range/v3/all.hpp>
#include <networking/downloader.hpp>

#include <QCoreApplication>

using namespace ranges;
using namespace std::chrono_literals;

auto main(int argc, char **argv) -> int {
    QCoreApplication application(argc, argv);
    auto console = spdlog::stdout_color_mt("console");

    network::downloader dl;
    dl.download({"https://api.alternative.me/fng/"}, [&](const QByteArray &data) {
        spdlog::info("json: {}", QString::fromUtf8(data).toStdString());
        application.quit();
    });

    application.exec();

    return 0;
}
