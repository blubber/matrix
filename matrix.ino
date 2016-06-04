/*
    Data Format
    ~~~~~~~~~~~

    Each frame of sensor data consists for three bytes, the first
    byte is a ``channel`` byte, the latter two are ``data`` bytes.

    Channel is a 6-bit (0 - 63) integer, data is a 10-bit (0-1023)
    integer.

    Each byte is identified by its LSB and MSB value, see table below::


            CHANNEL              MSB                  LSB
        ---------------  |  ---------------  |  ---------------
        7 6 5 4 3 2 1 0  |  7 6 5 4 3 2 1 0  |  7 6 5 4 3 2 1 0
        1 C C C C C C 0  |  0 0 0 M M M M 0     0 L L L L L L 1

    Legend: 
      - C : Channel bits;
      - M : Most significant data bits;
      - L : Least significant data bits.
*/

int sig = 7;

int row_s0 = 12;
int row_s1 = 11;
int row_s2 = 10;

int column_s0 = 16;
int column_s1 = 15;
int column_s2 = 14;
int column_s3 = 13;

int row_mapping[] =  { 0, 2, 4, 6, 1, 3, 5, 7 };
int col_mapping[] =  { 5, 0, 4, 2, 1, 3, 6, 7 };


void setup() {
    Serial.begin(38400);
    pinMode(sig, INPUT);
    
    pinMode(row_s0, OUTPUT);
    pinMode(row_s1, OUTPUT);
    pinMode(row_s2, OUTPUT);

    pinMode(column_s0, OUTPUT);
    pinMode(column_s1, OUTPUT);
    pinMode(column_s2, OUTPUT);
    pinMode(column_s3, OUTPUT);

    digitalWrite (row_s0, LOW);
    digitalWrite (row_s1, LOW);
    digitalWrite (row_s2, LOW);

    digitalWrite (column_s0, LOW);
    digitalWrite (column_s1, LOW);
    digitalWrite (column_s2, LOW);
    digitalWrite (column_s3, LOW);

    digitalWrite (sig, LOW);
    
    Serial.flush();
}


void select_row (unsigned char c) {
    digitalWrite(row_s0, c & 0x1);
    digitalWrite(row_s1, (c >> 1) & 0x1);
    digitalWrite(row_s2, (c >> 2) & 0x1);
}

void select_column (unsigned char c) {
    digitalWrite(column_s0, c & 0x1);
    digitalWrite(column_s1, (c >> 1) & 0x1);
    digitalWrite(column_s2, (c >> 2) & 0x1);
    digitalWrite(column_s3, (c >> 3) & 0x1);
}

void loop() {
    unsigned short val = 0;
    unsigned char frame[3];
    unsigned char row = 0;
    unsigned char column = 0;

    for (row = 0; row < 8; row++){
        select_row(row);

        for (column = 0; column < 8; column++) {
	        select_column(column + 8);
            delayMicroseconds(500);

            val = analogRead(sig);

            frame[0] = 0x80 | (((row_mapping[row] * 8) + col_mapping[(column)]) << 1);
            frame[1] = (val >> 5) & 0x1e;
            frame[2] = ((val & 0x3f) << 1) + 1;

            Serial.write(frame, 3);
        }
    }
}

