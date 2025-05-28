// Rubiks Cube Solver 0.3
// Judah Keelin
// 10-21-2024



// Adjustable variables

// Number of steps per turn
const int stepsPerTurn = 50;

// Delay between motor steps (the smaller this number is the faster the rotation)
const int stepDelay = 400;

// Delay between turns
const int delayBetweenTurns = 50;

// Spin direction value for dir pin, may change depending on final motor pinout
const int clockwise = 1;
const int counterClockwise = 0;

// Solution string for testing purposes
// Final program will use a serial connection to retrieve the solution from the raspberry pi
const String solutionString = "R2F1L1B3D1U3R1L3D2F2B2R3U2F3D2L2B1F1D2U2";
// R2F1L1B3D1U3R1L3D2F2B2R3U2F3D2L2B1F1D2U2
// D2L3D3L2U1R2F1B1L1B1D3B2R2U3R2U3F2R2U3L2
// U1R3D3L1B1F1U2L3F2R2D1U3B3R1L2U2D2L2B3R1
// D2U2L1B2U3B1F2U2R3U1F2U2D2F1R2D3F2U1D2L1

// Non adjustable variables

// Maximum number of steps a solution will ever have
const int maxSolutionLength = 30;

int interpretation[maxSolutionLength];  // Array to store the move integers
int moveCount = 0;         // Counter for moves stored

// Output Pin Declaration
const int dirPinR = 2;
const int stepPinR = 3;

const int dirPinL = 4;
const int stepPinL = 5;

const int dirPinF = 6;
const int stepPinF = 7;

const int dirPinB = 8;
const int stepPinB = 9;

const int dirPinU = 10;
const int stepPinU = 11;

const int dirPinD = 12;
const int stepPinD = 13;

void setup() {
  // Create serial output
  Serial.begin(9600);

  // Declare pins as outputs
  pinMode(dirPinR,OUTPUT);
  pinMode(stepPinR,OUTPUT);

  pinMode(dirPinL,OUTPUT);
  pinMode(stepPinL,OUTPUT);

  pinMode(dirPinF,OUTPUT);
  pinMode(stepPinF,OUTPUT);

  pinMode(dirPinB,OUTPUT);
  pinMode(stepPinB,OUTPUT);

  pinMode(dirPinU,OUTPUT);
  pinMode(stepPinU,OUTPUT);

  pinMode(dirPinD,OUTPUT);
  pinMode(stepPinD,OUTPUT); 

  // Interpretate
  processSolutionString(solutionString);

  Serial.println(solutionString);
  // Output the parsed moves
  Serial.println("Parsed Moves:");
  for (int i = 0; i < moveCount; i++) {
    Serial.print(String(interpretation[i]) + ',');
  }
  Serial.println();

  // Start time
  unsigned long startTime = millis();

  // Solving
  runSolution(interpretation);

  // End Time
  unsigned long endTime = millis();

  // Output time
  Serial.println("Total Time: " + String((endTime - startTime)/1000.0000) + "sec");
}

void loop() {}

// Turns the specified motor a specified amount of times
// dirPin: direction pin of the specified motor
// stepPin; step pin of the specified motor
// turnCount: the number of turns for the function to execute (counter-clockwise turns are negative)
void turn (int dirPin, int stepPin, int turnCount) {
   // Set rotation direction
  if (turnCount >= 0) {
    digitalWrite(dirPin, clockwise);
  } else {
    digitalWrite(dirPin, counterClockwise);
  }

  // The actual steping of the motor
  for(int x = 0; x < (stepsPerTurn * abs(turnCount)); x++) {
    digitalWrite(stepPin,HIGH);
    delayMicroseconds(stepDelay);
    digitalWrite(stepPin,LOW); 
    delayMicroseconds(stepDelay);
  }
}

// Turns the specified motor a specified amount of times
// dirPin: direction pin of the specified motor
// stepPin; step pin of the specified motor
// turnCount: the number of turns for the function to execute (counter-clockwise turns are negative)
void parallelTurn (int dirPin1, int stepPin1, int turnCount1, int dirPin2, int stepPin2, int turnCount2) {
   // Set rotation direction
  if (turnCount1 >= 0) {
    digitalWrite(dirPin1, clockwise);
  } else {
    digitalWrite(dirPin1, counterClockwise);
  }

  if (turnCount2 >= 0) {
    digitalWrite(dirPin2, clockwise);
  } else {
    digitalWrite(dirPin2, counterClockwise);
  }

  int motor1Index = 0;
  int motor2Index = 0;

  while (motor1Index < abs(turnCount1) * stepsPerTurn && motor2Index < abs(turnCount2) * stepsPerTurn) {
    digitalWrite(stepPin1,HIGH);
    digitalWrite(stepPin2,HIGH);
    delayMicroseconds(stepDelay);
    digitalWrite(stepPin1,LOW);
    digitalWrite(stepPin2,LOW);
    delayMicroseconds(stepDelay);
    motor1Index++;
    motor2Index++;
  }

  while (motor1Index < abs(turnCount1) * stepsPerTurn) {
    digitalWrite(stepPin1,HIGH);
    delayMicroseconds(stepDelay);
    digitalWrite(stepPin1,LOW); 
    delayMicroseconds(stepDelay);
    motor1Index++;
  }

  while (motor2Index < abs(turnCount2) * stepsPerTurn) {
    digitalWrite(stepPin2,HIGH);
    delayMicroseconds(stepDelay);
    digitalWrite(stepPin2,LOW); 
    delayMicroseconds(stepDelay);
    motor2Index++;
  }
}

// Lookup function to map face and rotation to integer
int getMoveValue(char face, int rotation) {
  switch (face) {
    case 'R': return rotation;               // R1 = 1, R2 = 2, R3 = 3
    case 'L': return 3 + rotation;           // L1 = 4, L2 = 5, L3 = 6
    case 'F': return 6 + rotation;           // F1 = 7, F2 = 8, F3 = 9
    case 'B': return 9 + rotation;           // B1 = 10, B2 = 11, B3 = 12
    case 'U': return 12 + rotation;          // U1 = 13, U2 = 14, U3 = 15
    case 'D': return 15 + rotation;          // D1 = 16, D2 = 17, D3 = 18
  }
  return -1;  // Invalid move
}

// Function to check for and process adjacent combinations
int checkForCombo(char face1, int rotation1, char face2, int rotation2) {
  // R-L combinations
  if ((face1 == 'R' && face2 == 'L') || (face1 == 'L' && face2 == 'R')) {
    if (rotation1 == 1 && rotation2 == 1) return 19; // R1, L1 or L1, R1
    if (rotation1 == 2 && rotation2 == 2) return 20; // R2, L2 or L2, R2
    if (rotation1 == 3 && rotation2 == 3) return 21; // R3, L3 or L3, R3
    if (rotation1 == 1 && rotation2 == 2) return 23; // R1, L2 or L2, R1
    if (rotation1 == 2 && rotation2 == 1) return 22; // R2, L1 or L1, R2
    if (rotation1 == 3 && rotation2 == 1) return 24; // R3, L1 or L1, R3
    if (rotation1 == 1 && rotation2 == 3) return 25; // R1, L3 or L3, R1
  }
  // U-D combinations
  if ((face1 == 'U' && face2 == 'D') || (face1 == 'D' && face2 == 'U')) {
    if (rotation1 == 1 && rotation2 == 1) return 26; // U1, D1 or D1, U1
    if (rotation1 == 2 && rotation2 == 2) return 27; // U2, D2 or D2, U2
    if (rotation1 == 3 && rotation2 == 3) return 28; // U3, D3 or D3, U3
    if (rotation1 == 1 && rotation2 == 2) return 29; // U1, D2 or D2, U1
    if (rotation1 == 2 && rotation2 == 1) return 30; // U2, D1 or D1, U2
    if (rotation1 == 3 && rotation2 == 1) return 31; // U3, D1 or D1, U3
    if (rotation1 == 1 && rotation2 == 3) return 32; // U1, D3 or D3, U1
  }
  // F-B combinations
  if ((face1 == 'F' && face2 == 'B') || (face1 == 'B' && face2 == 'F')) {
    if (rotation1 == 1 && rotation2 == 1) return 33; // F1, B1 or B1, F1
    if (rotation1 == 2 && rotation2 == 2) return 34; // F2, B2 or B2, F2
    if (rotation1 == 3 && rotation2 == 3) return 35; // F3, B3 or B3, F3
    if (rotation1 == 1 && rotation2 == 2) return 36; // F1, B2 or B2, F1
    if (rotation1 == 2 && rotation2 == 1) return 37; // F2, B1 or B1, F2
    if (rotation1 == 3 && rotation2 == 1) return 38; // F3, B1 or B1, F3
    if (rotation1 == 1 && rotation2 == 3) return 39; // F1, B3 or B3, F1
  }
  
  return -1;  // No combination found
}

// Function to process the entire string and populate the move array
void processSolutionString(String solution) {
  moveCount = 0;  // Reset move counter
  int i = 0;      // Index for string parsing
  
  while (i < solution.length()) {
    // Get the current face and rotation
    char face1 = solution[i];
    int rotation1 = solution[i + 1] - '0';  // Convert char to int
    
    // Check if there's an adjacent command for combination
    if (i + 3 < solution.length()) {
      char face2 = solution[i + 2];
      int rotation2 = solution[i + 3] - '0';
      
      // Check for combinations
      int comboValue = checkForCombo(face1, rotation1, face2, rotation2);
      if (comboValue != -1) {
        interpretation[moveCount++] = comboValue;
        i += 4;  // Skip the next command since it's part of a combination
        continue;
      }
    }
    
    // If no combination, handle the single move
    int moveValue = getMoveValue(face1, rotation1);
    if (moveValue != -1) {
      interpretation[moveCount++] = moveValue;
    }
    i += 2;  // Move to the next face-rotation pair
  }
}



// Executes the solution
// interpretation[]: a reference to the array that holds the final interpretation of the solution
void runSolution(int interpretation[]) {
  int index = 0;

  while (interpretation[index] > 0) {
    if (index != 0) {
      delay(delayBetweenTurns);
    }

    switch (interpretation[index]) {
      case 1: {
        turn(dirPinR, stepPinR, 1);
        break;
      }
      case 2: {
        turn(dirPinR, stepPinR, 2);
        break;
      }
      case 3: {
        turn(dirPinR, stepPinR, -1);
        break;
      }
      case 4: {
        turn(dirPinL, stepPinL, 1);
        break;
      }
      case 5: {
        turn(dirPinL, stepPinL, 2);
        break;
      }
      case 6: {
        turn(dirPinL, stepPinL, -1);
        break;
      }
      case 7: {
        turn(dirPinF, stepPinF, 1);
        break;
      }
      case 8: {
        turn(dirPinF, stepPinF, 2);
        break;
      }
      case 9: {
        turn(dirPinF, stepPinF, -1);
        break;
      }
      case 10: {
        turn(dirPinB, stepPinB, 1);
        break;
      }
      case 11: {
        turn(dirPinB, stepPinB, 2);
        break;
      }
      case 12: {
        turn(dirPinB, stepPinB, -1);
        break;
      }
      case 13: {
        turn(dirPinU, stepPinU, 1);
        break;
      }
      case 14: {
        turn(dirPinU, stepPinU, 2);
        break;
      }
      case 15: {
        turn(dirPinU, stepPinU, -1);
        break;
      }
      case 16: {
        turn(dirPinD, stepPinD, 1);
        break;
      }
      case 17: {
        turn(dirPinD, stepPinD, 2);
        break;
      }
      case 18: {
        turn(dirPinD, stepPinD, -1);
        break;
      }
      case 19: {
        parallelTurn(dirPinR, stepPinR, 1, dirPinL, stepPinL, 1);
        break;
      }
      case 20: {
        parallelTurn(dirPinR, stepPinR, 2, dirPinL, stepPinL, 2);
        break;
      }
      case 21: {
        parallelTurn(dirPinR, stepPinR, -1, dirPinL, stepPinL, -1);
        break;
      }
      case 22: {
        parallelTurn(dirPinR, stepPinR, 1, dirPinL, stepPinL, 2);
        break;
      }
      case 23: {
        parallelTurn(dirPinR, stepPinR, 2, dirPinL, stepPinL, 1);
        break;
      }
      case 24: {
        parallelTurn(dirPinR, stepPinR, -1, dirPinL, stepPinL, 1);
        break;
      }
      case 25: {
        parallelTurn(dirPinR, stepPinR, 1, dirPinL, stepPinL, -1);
        break;
      }
      case 26: {
        parallelTurn(dirPinU, stepPinU, 1, dirPinD, stepPinD, 1);
        break;
      }
      case 27: {
        parallelTurn(dirPinU, stepPinU, 2, dirPinD, stepPinD, 2);
        break;
      }
      case 28: {
        parallelTurn(dirPinU, stepPinU, -1, dirPinD, stepPinD, -1);
        break;
      }
      case 29: {
        parallelTurn(dirPinU, stepPinU, 1, dirPinD, stepPinD, 2);
        break;
      }
      case 30: {
        parallelTurn(dirPinU, stepPinU, 2, dirPinD, stepPinD, 1);
        break;
      }
      case 31: {
        parallelTurn(dirPinU, stepPinU, -1, dirPinD, stepPinD, 1);
        break;
      }
      case 32: {
        parallelTurn(dirPinU, stepPinU, 1, dirPinD, stepPinD, -1);
        break;
      }
      case 33: {
        parallelTurn(dirPinF, stepPinF, 1, dirPinB, stepPinB, 1);
        break;
      }
      case 34: {
        parallelTurn(dirPinF, stepPinF, 2, dirPinB, stepPinB, 2);
        break;
      }
      case 35: {
        parallelTurn(dirPinF, stepPinF, -1, dirPinB, stepPinB, -1);
        break;
      }
      case 36: {
        parallelTurn(dirPinF, stepPinF, 1, dirPinB, stepPinB, 2);
        break;
      }
      case 37: {
        parallelTurn(dirPinF, stepPinF, 2, dirPinB, stepPinB, 1);
        break;
      }
      case 38: {
        parallelTurn(dirPinF, stepPinF, -1, dirPinB, stepPinB, 1);
        break;
      }
      case 39: {
        parallelTurn(dirPinF, stepPinF, 1, dirPinB, stepPinB, -1);
        break;
      }
      default: {
        Serial.println("Unknown Case");
        break;
      }
    }

    index++;
  }
}