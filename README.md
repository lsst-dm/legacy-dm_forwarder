# dm_Forwarder
Forwarder application for transferring fitsfiles

## How to set redis password
1. Manually Change Configuration File
   * In redis.conf uncomment the line that says #requirepass 
2. Command Line
   * `redis-cli config set requirepass <your_password>`