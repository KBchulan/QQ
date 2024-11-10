#include "AvatarUploader.h"
#include <filesystem>
#include <fstream>
#include <sstream>

namespace fs = std::filesystem;

AvatarInfo AvatarUploader::processAvatar(const std::string& filePath) {
    AvatarInfo info;
    
    // 验证文件
    if (!validateImage(filePath)) {
        throw std::runtime_error("Invalid image file");
    }
    
    // 读取原始图片
    cv::Mat original = cv::imread(filePath);
    if (original.empty()) {
        throw std::runtime_error("Failed to load image");
    }
    
    // 创建缩略图
    cv::Mat thumbnail = createThumbnail(original);
    
    // 生成文件路径
    std::string filename = fs::path(filePath).filename().string();
    std::string timestamp = std::to_string(std::time(nullptr));
    
    info.originalPath = "uploads/avatars/original_" + timestamp + "_" + filename;
    info.thumbnailPath = "uploads/avatars/thumb_" + timestamp + "_" + filename;
    info.mimeType = getMimeType(filePath);
    info.size = fs::file_size(filePath);
    
    // 保存图片
    cv::imwrite(info.originalPath, original);
    cv::imwrite(info.thumbnailPath, thumbnail);
    
    return info;
}

cv::Mat AvatarUploader::createThumbnail(const cv::Mat& original) {
    cv::Mat thumbnail;
    double scale = std::min(
        static_cast<double>(THUMBNAIL_SIZE) / original.cols,
        static_cast<double>(THUMBNAIL_SIZE) / original.rows
    );
    
    cv::resize(original, thumbnail, cv::Size(), scale, scale, cv::INTER_AREA);
    return thumbnail;
}

std::string AvatarUploader::getMimeType(const std::string& filePath) {
    std::string ext = fs::path(filePath).extension().string();
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
    
    if (ext == ".jpg" || ext == ".jpeg") return "image/jpeg";
    if (ext == ".png") return "image/png";
    if (ext == ".gif") return "image/gif";
    return "application/octet-stream";
}

bool AvatarUploader::validateImage(const std::string& filePath) {
    // 检查文件大小
    if (fs::file_size(filePath) > MAX_SIZE) {
        return false;
    }
    
    // 检查文件类型
    cv::Mat img = cv::imread(filePath);
    if (img.empty()) {
        return false;
    }
    
    // 检查图片尺寸
    if (img.cols > 4096 || img.rows > 4096) {
        return false;
    }
    
    return true;
}

bool AvatarUploader::uploadAvatar(const AvatarInfo& info, int64_t userId) {
    // TODO: 实现数据库更新
    return true;
}

std::string AvatarUploader::getAvatarUrl(int64_t userId) {
    // TODO: 从数据库获取用户头像URL
    return "default_avatar.png";
} 