from .acceleration import AccelerationScenario

SCENARIOS = {
    AccelerationScenario.name: AccelerationScenario,
}

__all__ = ["SCENARIOS", "AccelerationScenario"]
