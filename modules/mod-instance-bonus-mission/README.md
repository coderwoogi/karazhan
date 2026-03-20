# mod-instance-bonus-mission

This module adds bonus mission content for dungeon and raid instances.

Core responsibilities:
- detect party entry into an instance
- load candidate missions for the instance
- create one active mission per instance
- track progress and time limits
- announce progress and failure states
- reward the party when the mission is completed

The local LLM is only responsible for mission selection and briefing text.
The AzerothCore module is responsible for all gameplay rules.