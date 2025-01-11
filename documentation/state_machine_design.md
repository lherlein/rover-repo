# State Machine Design

The rover is a large state machine, and the state it's in at any time drive behavior. There are many states, most having substates. The individual state diagrams can be found in their respective docs, but the point of this document is the entire state machine of the rover. 

```mermaid
stateDiagram-v2
  state Setup {
    Access_Point_Mode --> Ground_Station_Setup: User Connection && Response
    Ground_Station_Setup
  }
  state Active {
    Standby --> Standby: Listening
    state ManualControl {
      Still --> Moving: Command Given
      Moving --> Still
    }
    state AutonomousControl {
      TBD
    }
    Standby --> ManualControl: MANUAL signal
    Standby --> AutonomousControl: AUTO signal
  }
  Ground_Station_Setup --> Standby: Successful Handshake
  Ground_Station_Setup --> Access_Point_Mode: Failed Handshake
  Standby --> Setup: 5 or more Heartbeats lost
```
