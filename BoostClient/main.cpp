#include <iostream>
#include <boost/asio.hpp>
#include <opencv2/opencv.hpp>
#include <boost/asio/ssl.hpp>
#include <Windows.h>
#include <set>

#include "Header.h"
#include "AckHeader.h"
#include "KeyspaceMonitor.h"
#include "ScreenLocker.h"

using namespace std;

ScreenLocker* lockscreen_handler = new ScreenLocker();
KeyspaceMonitor* monitor = new KeyspaceMonitor(lockscreen_handler);

//const std::string kServerIpAddress = "192.168.122.246";
const std::string kServerIpAddress = "192.168.122.1";
const unsigned short kServerPort = 12345;

bool lockRequestInProgress = false;
bool unlockRequestInProgress = false;

void readHeader(std::shared_ptr<boost::asio::ssl::stream<boost::asio::ip::tcp::socket>>& socket_ptr, ScreenLocker* lockscreen_handler);
void readTextData(std::shared_ptr<boost::asio::ssl::stream<boost::asio::ip::tcp::socket>>& socket_, Header& header_);
void sendScreenshot(std::shared_ptr<boost::asio::ssl::stream<boost::asio::ip::tcp::socket>>& socket);

void lockScreen(ScreenLocker* lockscreen_handle) {
    if (!lockscreen_handler->isLocked() && !lockRequestInProgress && !unlockRequestInProgress) {
        lockRequestInProgress = true;
        lockscreen_handle->runInThread();
        lockRequestInProgress = false;
    }
}

void unlockScreen(ScreenLocker* lockscreen_handle) {
    if (lockscreen_handler->isLocked() && !lockRequestInProgress && !unlockRequestInProgress) {
        unlockRequestInProgress = true;
        lockscreen_handle->closeWindow();
        lockscreen_handle->waitForThread();
        unlockRequestInProgress = false;
    }
}

void removeFromWhitelist(std::shared_ptr<boost::asio::ssl::stream<boost::asio::ip::tcp::socket>>& socket_) {
    std::array<char, 1024> word{};

    boost::asio::read(*socket_, boost::asio::buffer(&word, sizeof(word)));

    std::string toString;

    for (auto& ch : word) {
        if (ch == '\0')
            break;
        toString += ch;
    }

    monitor->removeFromBlacklist(toString);
}

void addToWhitelist(std::shared_ptr<boost::asio::ssl::stream<boost::asio::ip::tcp::socket>>& socket_) {
    std::array<char, 1024> word{};

    boost::asio::read(*socket_, boost::asio::buffer(&word, sizeof(word)));

    std::string toString;

    for (auto& ch : word) {
        if (ch == '\0')
            break;
        toString += ch;
    }

    monitor->addToBlacklist(toString);
}

std::vector<u_char> takeScreenshot()
{
    // Get the dimensions of the desktop.
    HDC hdcScreen = GetDC(NULL);
    int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = GetSystemMetrics(SM_CYSCREEN);

    // Create a device context for the screenshot.
    HDC hdcMem = CreateCompatibleDC(hdcScreen);
    HBITMAP hBitmap = CreateCompatibleBitmap(hdcScreen, screenWidth, screenHeight);
    SelectObject(hdcMem, hBitmap);

    // Take the screenshot and copy it to the device context.
    BitBlt(hdcMem, 0, 0, screenWidth, screenHeight, hdcScreen, 0, 0, SRCCOPY);

    // Create an OpenCV Mat from the screenshot.
    BITMAPINFOHEADER bi;
    bi.biSize = sizeof(BITMAPINFOHEADER);
    bi.biWidth = screenWidth;
    bi.biHeight = -screenHeight; // negative height to flip image
    bi.biPlanes = 1;
    bi.biBitCount = 24;
    bi.biCompression = BI_RGB;
    bi.biSizeImage = 0;
    bi.biXPelsPerMeter = 0;
    bi.biYPelsPerMeter = 0;
    bi.biClrUsed = 0;
    bi.biClrImportant = 0;
    cv::Mat screenshotMat = cv::Mat(screenHeight, screenWidth, CV_8UC3);
    GetDIBits(hdcScreen, hBitmap, 0, screenHeight, screenshotMat.data, (BITMAPINFO*)&bi, DIB_RGB_COLORS);

    // Compress the screenshot as a PNG using OpenCV.
    std::vector<uchar> compressedData;
    cv::imencode(".png", screenshotMat, compressedData);

    // Clean up.
    DeleteObject(hBitmap);
    DeleteDC(hdcMem);
    ReleaseDC(NULL, hdcScreen);

    // Return the compressed image data.
    return compressedData;
}

void readHeader(std::shared_ptr<boost::asio::ssl::stream<boost::asio::ip::tcp::socket>>& socket_ptr, ScreenLocker* lockscreen_handler) {
    while (true) {
        try {
            std::cout << "WAIT FOR PACKET" << std::endl;

            Header header_;

            boost::asio::read(*socket_ptr, boost::asio::buffer(&header_, sizeof(Header)));

            std::cout << "PACKET RECIEVED: " << header_.type << std::endl;
            switch (header_.type) {
            case TEXT:
                readTextData(socket_ptr, header_);
                break;
            case LOCK_SCREEN:
                lockScreen(lockscreen_handler);
                break;
            case UNLOCK_SCREEN:
                unlockScreen(lockscreen_handler);
                break;
            case SCREENSHOT:
                sendScreenshot(socket_ptr);
                break;
            case SCREENSHOT_REQ:
                sendScreenshot(socket_ptr);
                break;
            case REMOVE_FROM_WHITELIST:
                removeFromWhitelist(socket_ptr);
                break;
            case ADD_TO_WHITELIST:
                addToWhitelist(socket_ptr);
                break;
            default:
                throw std::exception("Unknown data type received");
                break;
            }
        }
        catch ( std::exception &e) {
            std::cerr << e.what() << std::endl;
            break;
        }
    }
}

void readTextData(std::shared_ptr<boost::asio::ssl::stream<boost::asio::ip::tcp::socket>>& socket_, Header& header_) {
    try {
        std::array<char, 1024> text_buffer_;

        std::cout << "WAIT FOR MESSAGE" << std::endl;
        boost::asio::read(*socket_, boost::asio::buffer(text_buffer_, header_.size));
        std::cout << "RECIEVED MESSAGE" << " WITH SIZE " << header_.size << std::endl;
        cout << text_buffer_.data() << endl;
    }
    catch (std::exception& ex) {
        cerr << "ERROR: MSG READ" << ex.what() << endl;
    }
}

void sendText(boost::asio::ip::tcp::socket& socket) {
    std::string text;
    while (std::getline(std::cin, text)) {
        Header header;
        header.type = TEXT;
        header.size = text.size();

        boost::asio::write(socket, boost::asio::buffer(&header, sizeof(header)));
        boost::asio::write(socket, boost::asio::buffer(text));
    }
}

void sendScreenshot(std::shared_ptr<boost::asio::ssl::stream<boost::asio::ip::tcp::socket>>& socket) {
    try {
        MetaData metadata;
        metadata.is_locked = lockscreen_handler->isLocked();
        metadata.keyData = {};
        metadata.BlacklistData = {};
        std::vector<u_char> tempLog;

        monitor->getLog(tempLog);

        if (tempLog.size() <= metadata.keyData.size()) {
            std::copy(tempLog.begin(), tempLog.end(), metadata.keyData.begin());
        }
        else {
            std::cerr << "The array is too small to store all elements from the vector." << std::endl;
        }

        size_t currentIndex = 0;

        for (const auto& str : monitor->getBlacklist()) {
            for (char c : str) {
                metadata.BlacklistData[currentIndex++] = static_cast<unsigned char>(c);
            }
            metadata.BlacklistData[currentIndex++] = ' ';
        }

        std::vector<unsigned char> screenshot_data = takeScreenshot();


        Header header;
        header.type = SCREENSHOT;
        header.size = screenshot_data.size();
        header.meta_length = sizeof(metadata);

        std::cout << "SCREENSHOT SIZE: " << (float)(header.size / 1000) / 1000 << "MB" << std::endl;

        boost::asio::write(*socket, boost::asio::buffer(&header, sizeof(header)));
        const size_t chunk_size = 1024; // adjust this

        size_t bytes_sent = 0;
        while (bytes_sent < screenshot_data.size()) {
            try {
                size_t bytes_to_send = std::min(chunk_size, screenshot_data.size() - bytes_sent);
                boost::asio::write(*socket, boost::asio::buffer(&screenshot_data[bytes_sent], bytes_to_send));
                bytes_sent += bytes_to_send;
            }
            catch ( std::exception &e ) {
                std::cerr << e.what() << std::endl;
                break;
            }
        }
        //send metadata
        cout << "SENDING METADATA: Locked? :" << (int)lockscreen_handler->isLocked() << ": SIZE OF: " << header.meta_length << endl;
        boost::asio::write(*socket, boost::asio::buffer(&metadata, sizeof(metadata)));

        Ack ack;
        boost::asio::read(*socket, boost::asio::buffer(&ack, sizeof(ack)));

        if (ack.status == RECEIVED) {
            cout << "Reply: Screenshot Recieved" << endl;
        }
        else {
            cerr << "ERROR: Screenshot Not Recieved" << endl;
        }
    }
    catch ( ... ) {
        throw;
    }
}

std::shared_ptr<boost::asio::ssl::stream<boost::asio::ip::tcp::socket>> connect_to_server(boost::asio::io_context& io_context, boost::asio::ssl::context& ssl_context) {
    std::shared_ptr<boost::asio::ssl::stream<boost::asio::ip::tcp::socket>> socket_ptr(new boost::asio::ssl::stream<boost::asio::ip::tcp::socket>(io_context, ssl_context));

    boost::asio::ip::tcp::resolver resolver(io_context);
    auto endpoints = resolver.resolve(kServerIpAddress, std::to_string(kServerPort));
    boost::asio::connect(socket_ptr->lowest_layer(), endpoints);

    socket_ptr->handshake(boost::asio::ssl::stream_base::client);  // Perform SSL handshake

    return socket_ptr;
}

int main() {
    try {
        // Add SSL context
        
        boost::asio::ssl::context ssl_context(boost::asio::ssl::context::tlsv12_client);
        try {
            ssl_context.load_verify_file("ca.crt");
            ssl_context.use_certificate_chain_file("client.crt");
            ssl_context.use_private_key_file("client.key", boost::asio::ssl::context::pem);
        }
        catch (const boost::system::system_error& ex) {
            std::cerr << "SSL context initialization failed: " << ex.what() << std::endl;
        }

        while (true) {
            try {
                boost::asio::io_context io_context;

                // Pass the ssl_context to the connect_to_server function
                std::shared_ptr<boost::asio::ssl::stream<boost::asio::ip::tcp::socket>> socket_ptr = connect_to_server(io_context, ssl_context);

                // Update the argument type for the readHeader_thread
                std::thread readHeader_thread(readHeader, std::ref(socket_ptr), lockscreen_handler);

                unlockScreen(lockscreen_handler);

                readHeader_thread.join();
            }
            catch (const std::exception& e) {
                std::cerr << "Error: " << e.what() << std::endl;

                //lockScreen(lockscreen_handler);

                std::this_thread::sleep_for(std::chrono::seconds(5));
            }
        }
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
