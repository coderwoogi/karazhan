# ![logo](https://raw.githubusercontent.com/azerothcore/azerothcore.github.io/master/images/logo-github.png) AzerothCore
## Reward Played Time Improved
- Latest build status with azerothcore: [![core-build](https://github.com/freekode/mod-reward-played-time-improved/actions/workflows/core-build.yml/badge.svg)](https://github.com/freekode/mod-reward-played-time-improved/actions/workflows/core-build.yml)


This is a module for [AzerothCore](http://www.azerothcore.org) that adds items for players that have stayed logged in the x amount of time.

**Current features**

This module grants players a random reward item every some time of session playtime.
* Reward interval configurable, default is 1h
* Rewards are chosen from a configurable list
* Items are added directly to the player’s bags. If bags are full, the item is sent via in-game mail

List of Items: https://docs.google.com/spreadsheets/d/1ELCKMdPya1XaBS-YImPtIuEjEPXj2yf1AuVOHhf-QHs/edit?usp=sharing

**Upcoming features**

* Add amount for reward item
* Add different intervals and different list of items for each interval


## Requirements

Reward System module currently requires:

AzerothCore v1.0.1+

## How to install

1. Simply place the module under the `modules` folder of your AzerothCore source folder.
2. Re-run cmake and launch a clean build of AzerothCore

**That's it.**

### (Optional) Edit module configuration

If you need to change the module configuration, go to your server configuration folder (e.g. **etc**), copy `mod-reward-played-time-improved.conf.dist` to `mod-reward-played-time-improved.conf` and edit it as you prefer.

