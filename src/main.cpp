#include <iostream>
#include <string>
#include <vector>
#include <exception>
#include <functional>

#include <curl/curl.h>

#include <tgbot/tgbot.h>

std::vector<std::string> botCommandsList = {"start", "site", "latest_news"};

static size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

void botCommandStart(TgBot::Bot &bot);
void botCommandSite(TgBot::Bot &bot);
void botCommandLatestNews(TgBot::Bot &bot, std::string &latestNews);

int main()
{
    std::string token = "token";
    TgBot::Bot bot(token);

    CURL *curl;
    CURLcode res;
    std::string readBuffer;

    curl = curl_easy_init();
    if(curl)
    {
        curl_easy_setopt(curl, CURLOPT_URL, "https://www.binance.com/ru/support/announcement/c-48?navId=48");
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
        res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);
    }

    std::vector<std::string> latestNewsBuffer;
    std::string forFind = "code\":\"";
    size_t pos = readBuffer.find(forFind, 0);
    int count = 0;
    while (count < 3 || pos != std::string::npos)
    {
        size_t temp = pos + 7;
        latestNewsBuffer.push_back("");
        while (readBuffer[temp] != '\"')
        {
            latestNewsBuffer[count] += readBuffer[temp];
            temp++;
        }
        while (readBuffer[temp] != '\"')
        {
            latestNewsBuffer[count] += readBuffer[temp];
            temp++;
        }
        pos = readBuffer.find(forFind, pos + 1);
        count++;
    }
    bot.getApi().sendMessage("id", "test");
    // std::cout << readBuffer << std::endl;

    botCommandStart(bot);
    botCommandSite(bot);
    botCommandLatestNews(bot, latestNewsBuffer[0]);

    bot.getEvents().onAnyMessage([&bot](TgBot::Message::Ptr message)
    {
        for (const auto& commandFromList : botCommandsList)
        {
            if ('/' + commandFromList == message->text)
                return;
        }

        bot.getApi().sendMessage(message->chat->id, "Я всего лишь бот, вы слишком многого от меня хотите :(");
    });


    try {
        printf("Bot username: %s\n", bot.getApi().getMe()->username.c_str());
        bot.getApi().deleteWebhook();

        TgBot::TgLongPoll longPoll(bot);
        while (true) {
            printf("Long poll started\n");
            longPoll.start();
        }
    } catch (std::exception& e) {
        printf("error: %s\n", e.what());
    }


    return 0;
}

void botCommandStart(TgBot::Bot &bot)
{
    bot.getEvents().onCommand("start", [&bot](TgBot::Message::Ptr message)
    {
        bot.getApi().sendMessage(message->chat->id,
                                 "Привет! Я умею присылать уведомления о свежих новостях на сайте www.binance.com");
    });
}

void botCommandSite(TgBot::Bot &bot)
{
    bot.getEvents().onCommand("site", [&bot](TgBot::Message::Ptr message)
    {
        bot.getApi().sendMessage(message->chat->id,
                                 "Все новые крипто-листинги:\n"
                                 "https://www.binance.com/ru/support/announcement/c-48?navId=48");
    });
}

void botCommandLatestNews(TgBot::Bot &bot, std::string &latestNews)
{
    // std::cout << latestNews;
    bot.getEvents().onCommand("latest_news", [&bot, &latestNews](TgBot::Message::Ptr message)
    {
        bot.getApi().sendMessage(message->chat->id, "https://www.binance.com/ru/support/announcement/" + latestNews);
    });
}