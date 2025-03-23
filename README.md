# Map Server

This package contains a ROS node which acts as a `map_server` by loading map data from a YAML configuration file and publishing it as an occupancy grid. For fast map loading header-only library `stb_image` is used.


## Features

- Loads and processes map configuration and map image, publishing it as occupancy grid.
- same CLI usage as ROS 1 `map_server`
- compatibility with map-config files for ROS 1 `map_server`and ROS 2 `nav2_map_server` 
- Handles relative and absolute paths to map image in YAML.
- supports PGM image type for compatibility
- supports PNG image types and converts multiple channels (including alpha) to single channel
- fast availability of loaded maps
- tested under humble and jazzy

## Performance

A key motivation for creation of this package is the noticed discrepancy in image loading times between ROS 1 and ROS 2 implementations of `map_server` within navigation stack. By using `stb_image` library image load time could be reduced significantly comparing to ROS 1 `map_server` and ROS 2 `nav2_map_server`. Even large maps of 24.000 x 24.000 pixel size are loaded within seconds, where `nav2_map_server` takes over one minute to load.

## Installation

1. **Clone the repository**:
   ```sh
   cd ~/ros2_ws/src
   git clone https://github.com/Sanoronas/map_server.git
   ```

2. **Build the package**: Make sure to build in Release-Mode  for optimal performance
   ```sh
   cd ~/ros2_ws
   colcon build --packages-select map_server --cmake-args -DCMAKE_BUILD_TYPE=Release
   ```

3. **Source the workspace**:
   ```sh
   source install/setup.bash
   ```

## Usage

**Prepare the YAML configuration file**:
Create a YAML file with the map metadata. For example, `map_config.yaml`:
```yaml
image: map_image.png
resolution: 0.05
origin: [0.0, 0.0, 0.0]
negate: 0
occupied_thresh: 0.65
free_thresh: 0.196
```

**Run the map_server node from command-line**:
```sh
ros2 run map_server map_server path/to/map_config.yaml
```

**Run the map_server node from launch file**:
```python
from launch import LaunchDescription
from launch_ros.actions import Node


def generate_launch_description():
    return LaunchDescription(
        [
            Node(
                package="map_server",
                executable="map_server",
                name="map_server",
                output="screen",
                arguments=["/path/to/map.yaml"],
            )
        ]
    )
```

## Node Details

- **Publisher**:
  - `map` (`nav_msgs/msg/OccupancyGrid`): Publishes the occupancy grid map.

- **Parameters**:
  - The node takes a single argument, which is the path to the YAML configuration file.


## Contributing

Contributions are welcome! Please open an issue or submit a pull request if you have any improvements or bug fixes.

## Acknowledgments

- This node is inspired by the original `map_server` in ROS 1.
- Thanks to the developers of `stb_image` for the excellent image loading module.


