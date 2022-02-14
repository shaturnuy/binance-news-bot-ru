#include <iostream>
#include <string>
#include <vector>
#include <exception>
#include <thread>
#include <chrono>
#include <functional>

#include <curl/curl.h>
#include <tgbot/tgbot.h>

static const int SizeOfLatestNewsBuffer = 3;

std::vector<std::string> botCommandsList = {"start", "site", "latest_news"};

void botCommandStart(TgBot::Bot &bot);
void botCommandSite(TgBot::Bot &bot);
void botCommandLatestNews(TgBot::Bot &bot, std::pair<std::string, std::string> &latestNews);

std::string getHtml();
static size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp);
std::string parseHtml(std::string &htmlSource, std::string forFind, size_t &currentPos);
void parser(std::string& htmlSource, std::vector<std::pair<std::string, std::string>> &latestNewsBuffer);
void checkNews(TgBot::Bot &bot, std::vector<std::pair<std::string, std::string>> & savedLatestNewsBuffer);

int main()
{
    std::string token = "token";
    TgBot::Bot bot(token);

    std::vector<std::pair<std::string, std::string>> latestNewsBuffer;
    std::string htmlSource = getHtml();

    parser(htmlSource, latestNewsBuffer);
    // std::cout << htmlSource << std::endl;
    std::cout << latestNewsBuffer[0].first << " : " << latestNewsBuffer[0].second << std::endl;

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

        // bot.getApi().sendMessage(message->chat->id, "Я всего лишь бот, вы слишком многого от меня хотите :(");
    });

    // for (auto i = 0; i < 3; i++)
    // {
    //     bot.getApi().sendMessage(-id, "test 30s");
    //     std::this_thread::sleep_for(std::chrono::milliseconds(30000));
    // }

    try {
        printf("Bot username: %s\n", bot.getApi().getMe()->username.c_str());
        bot.getApi().deleteWebhook();

        TgBot::TgLongPoll longPoll(bot);
        while (true) {
            printf("Long poll started\n");
            longPoll.start();
            checkNews(bot, latestNewsBuffer);
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

void botCommandLatestNews(TgBot::Bot &bot, std::pair<std::string, std::string> &latestNews)
{
    bot.getEvents().onCommand("latest_news", [&bot, &latestNews](TgBot::Message::Ptr message)
    {
        bot.getApi().sendMessage(message->chat->id, latestNews.second + "\nhttps://www.binance.com/ru/support/announcement/" + latestNews.first);
    });
}

std::string getHtml()
{
    CURL *curl;
    CURLcode res;
    std::string htmlBuffer;

    curl = curl_easy_init();
    if(curl)
    {
        curl_easy_setopt(curl, CURLOPT_URL, "https://www.binance.com/ru/support/announcement/c-48?navId=48");
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &htmlBuffer);
        res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);
    }
    return htmlBuffer;
}

std::string parseHtml(std::string &htmlSource, std::string forFind, size_t &currentPos)
{
    std::string resultText = "";
    forFind += "\":\"";
    currentPos = htmlSource.find(forFind, currentPos + 1) + forFind.size();

    while (htmlSource[currentPos] != '\"')
        resultText += htmlSource[currentPos++];

    return resultText;
}

static size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

void parser(std::string& htmlSource, std::vector<std::pair<std::string, std::string>> &latestNewsBuffer)
{
    size_t parserPos = 0;
    for (int currentNews = 0; currentNews < SizeOfLatestNewsBuffer; currentNews++)
    {
        std::string currentNewslink = parseHtml(htmlSource, "code", parserPos);
        std::string currentNewstitle = parseHtml(htmlSource, "title", parserPos);
        latestNewsBuffer.push_back(std::make_pair(currentNewslink, currentNewstitle));
    }
}

void checkNews(TgBot::Bot &bot, std::vector<std::pair<std::string, std::string>> &savedLatestNewsBuffer)
{
    std::vector<std::pair<std::string, std::string>> tempNewsBuffer;
    std::string htmlSource = getHtml();
    parser(htmlSource, tempNewsBuffer);

    if (tempNewsBuffer[0] != savedLatestNewsBuffer[0])
    {
        bot.getApi().sendMessage(-id,
        tempNewsBuffer[0].first + "\nhttps://www.binance.com/ru/support/announcement/" + tempNewsBuffer[0].first);
    }

    for (int currentNews = 0; currentNews != SizeOfLatestNewsBuffer; currentNews++)
    {
        savedLatestNewsBuffer[currentNews] = tempNewsBuffer[currentNews];
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(10000));
}