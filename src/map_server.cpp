// Copyright 2025 Sönke Prophet
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <yaml-cpp/yaml.h>

#include <cmath>
#include <filesystem>
#include <nav_msgs/msg/occupancy_grid.hpp>
#include <rclcpp/rclcpp.hpp>
#include <vector>
#include <wordexp.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

using namespace std::chrono_literals;

class MapServer : public rclcpp::Node
{
public:
  MapServer(const std::string & filename)
  : Node("map_server"), filename_(filename)
  {
    // Create a QoS profile with durability set to TRANSIENT_LOCAL
    rclcpp::QoS qos(rclcpp::KeepLast(1));
    qos.transient_local();

    map_pub_ = this->create_publisher<nav_msgs::msg::OccupancyGrid>("map", qos);
    RCLCPP_INFO(this->get_logger(), "Map server started..");

    // Load map information from YAML file
    map_metadata_ = loadMapMetadata(filename_);
    RCLCPP_INFO(
      this->get_logger(), "Loading image_file: %s", map_metadata_["image"].as<std::string>().c_str()
    );

    // Load map image and convert image to occupancy grid
    map_msg_ = loadMapImage(map_metadata_);
    map_msg_.header.frame_id = "map";
    map_msg_.header.stamp = this->now();
    RCLCPP_INFO(
      this->get_logger(), "Read map %s: %d X %d map @ %.2f m/cell",
      map_metadata_["image"].as<std::string>().c_str(),
      map_msg_.info.width,
      map_msg_.info.height,
      map_msg_.info.resolution);

    // Publish occupancy grid msg
    map_pub_->publish(map_msg_);
  }

private:
  YAML::Node loadMapMetadata(const std::string & filename)
  {
    YAML::Node map_metadata = YAML::LoadFile(filename);
    if (!map_metadata["image"] || !map_metadata["resolution"] || !map_metadata["origin"] ||
      !map_metadata["negate"] || !map_metadata["occupied_thresh"] || !map_metadata["free_thresh"])
    {
      throw std::runtime_error("Invalid content of map config " + filename);
    }

    // Make image path absolute
    std::string img_path = map_metadata["image"].as<std::string>();
    if (!std::filesystem::path(img_path).is_absolute()) {
      std::filesystem::path config_dir_abspath = std::filesystem::path(filename).parent_path();
      img_path = (config_dir_abspath / img_path).string();
      map_metadata["image"] = img_path;
    }

    return map_metadata;
  }

  nav_msgs::msg::OccupancyGrid loadMapImage(const YAML::Node & map_metadata)
  {
    std::string map_img_path = map_metadata["image"].as<std::string>();
    double resolution = map_metadata["resolution"].as<double>();
    std::vector<double> origin = map_metadata["origin"].as<std::vector<double>>();
    bool negate = (map_metadata["negate"].as<int>() != 0);
    double occupied_thresh = map_metadata["occupied_thresh"].as<double>();
    double free_thresh = map_metadata["free_thresh"].as<double>();

    int width, height, channels;
    unsigned char * img = stbi_load(map_img_path.c_str(), &width, &height, &channels, 1);
    if (!img) {
      throw std::runtime_error("Failed to load image: " + map_img_path);
    }

    if (negate) {
      for (int i = 0; i < width * height; ++i) {
        img[i] = 255 - img[i];
      }
    }

    // Flip the image vertically
    for (int y = 0; y < height / 2; ++y) {
      for (int x = 0; x < width; ++x) {
        std::swap(img[y * width + x], img[(height - y - 1) * width + x]);
      }
    }

    std::vector<int8_t> map_img(width * height, -1);
    for (int i = 0; i < width * height; ++i) {
      if (img[i] < 255 * (1 - occupied_thresh)) {
        map_img[i] = 100;
      } else if (img[i] > 255 * (1 - free_thresh)) {
        map_img[i] = 0;
      }
    }

    stbi_image_free(img);

    nav_msgs::msg::OccupancyGrid occupancy_grid_msg;
    occupancy_grid_msg.info.height = height;
    occupancy_grid_msg.info.width = width;
    occupancy_grid_msg.info.resolution = resolution;
    occupancy_grid_msg.info.origin.position.x = origin[0];
    occupancy_grid_msg.info.origin.position.y = origin[1];
    occupancy_grid_msg.info.origin.orientation.w = std::cos(origin[2] / 2);
    occupancy_grid_msg.info.origin.orientation.z = std::sin(origin[2] / 2);
    occupancy_grid_msg.info.map_load_time = this->now();

    occupancy_grid_msg.data = map_img;

    return occupancy_grid_msg;
  }

  std::string filename_;
  rclcpp::Publisher<nav_msgs::msg::OccupancyGrid>::SharedPtr map_pub_;
  YAML::Node map_metadata_;
  nav_msgs::msg::OccupancyGrid map_msg_;
};

std::string expandTilde(const std::string& path) {
  wordexp_t p;
  int result = wordexp(path.c_str(), &p, 0);
  if (result != 0) {
    throw std::runtime_error("Error when processing input path");
  } 
  std::string expandedPath(p.we_wordv[0]);
  wordfree(&p);
  return expandedPath;
}

int main(int argc, char ** argv)
{ 
  rclcpp::init(argc, argv);

  if (argc < 2) {
    RCLCPP_ERROR(rclcpp::get_logger("map_server"), "Usage: map_server <filename>");
    return 1;
  }

  // Expand potential leading tilde in the filename
  std::string filename = expandTilde(argv[1]);

  auto map_server = std::make_shared<MapServer>(filename);
  rclcpp::spin(map_server);
  rclcpp::shutdown();
  return 0;
}
