#pragma once
#include <string>
#include <opencv2/opencv.hpp>

class AvatarUploader {
public:
    static const int MAX_SIZE = 1024 * 1024;  // 1MB
    static const int THUMBNAIL_SIZE = 128;     // 缩略图尺寸

    struct AvatarInfo {
        std::string originalPath;
        std::string thumbnailPath;
        std::string mimeType;
        size_t size;
    };

    static AvatarInfo processAvatar(const std::string& filePath);
    static bool uploadAvatar(const AvatarInfo& info, int64_t userId);
    static std::string getAvatarUrl(int64_t userId);

private:
    static cv::Mat createThumbnail(const cv::Mat& original);
    static std::string getMimeType(const std::string& filePath);
    static bool validateImage(const std::string& filePath);
}; 