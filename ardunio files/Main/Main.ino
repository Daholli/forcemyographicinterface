#include <BasicLinearAlgebra.h>
#include <Wire.h>

using namespace BLA;

// system variables DO NOT CHANGE
bool startup = true;
bool startup_signal_sent = false;
uint8_t message_values[8];

// constants
const int led_pin = 0;
const int button_start_pin = 3;
const int i2c_transmissionbus = 8;

// communication constants
#define START_SIGNAL 234
#define STARTUP_END 255

float fakebuffer[2];
byte buff_I1C[12];

// user defined variables
const int num_principal_components = 2;
const int num_centroids = 3;

// ##############################################################################################################################################

// coefficient matrix
BLA::Matrix<8, 8> coeff = {0.153414078492893, 0.243051912839889, -0.391227845614739, 0.142247658751522, 0.384682744104955, -0.462523327803991, 0.244298057677644, 0.567896188770749,
                           0.193836756064961, 0.0655644619061737, -0.379429680693407, 0.480469331809817, 0.474075536456197, 0.405919519911652, -0.0316483992540871, -0.439079787561910,
                           0.0695188348521630, 0.294637102223248, -0.202331563608459, -0.214928383176278, -0.296551868127812, -0.00627285680293526, 0.773473574924887, -0.367396390479851,
                           0.156656091751684, 0.563573022923260, 0.0525957843669852, 0.157841224281111, -0.339965077424087, 0.560832451773721, -0.0887034284159969, 0.438390872499927,
                           0.196706453005735, 0.0188272810345514, 0.668939102440335, 0.613278357631126, -0.00872186014652904, -0.225206976305802, 0.288891865965644, -0.0557626656465336,
                           0.401130525699377, 0.415442639615176, 0.380575087513022, -0.510208443068345, 0.458913255519428, -0.0317623702729149, -0.142377211038116, -0.171668772489820,
                           0.774180683830335, -0.538318038557167, -0.0816465732495474, -0.122385353851825, -0.147156729610907, 0.156927741956751, 0.108741026135438, 0.176373440054675,
                           0.332509510417844, 0.263568076346201, -0.245532949166722, 0.162497965027928, -0.437515437165315, -0.479974770544267, -0.466511283154713, -0.306347778546684};

// centroids
BLA::Matrix<num_centroids, num_principal_components> centroids = {57.549064481318500, -2.847321366172702,
                                                                  -46.787854954253234, -5.919627058468517,
                                                                  -10.233391521845613, 7.360994999978692};

// mean shift of PCA
BLA::Matrix<8> mu = {111.7366727016697, 142.2760008046671, 74.423456045061360, 114.7851538925769, 84.396499698249850, 134.7710722188695, 164.2836451418226, 143.4411587205794};

// ##############################################################################################################################################

// matrix for future messages
BLA::Matrix<8> message_matrix;

// needed for transmitting floats over I2C
union u_usplit
{
    byte to_byte[4];
    float to_float;
} split;

void setup()
{
    pinMode(NINA_RESETN, OUTPUT);
    digitalWrite(NINA_GPIO0, HIGH);
    digitalWrite(NINA_RESETN, HIGH);
    digitalWrite(NINA_RESETN, LOW);
    delay(100);
    digitalWrite(NINA_RESETN, HIGH);
    delay(100);
    digitalWrite(NINA_RESETN, LOW);

    pinMode(button_start_pin, INPUT);
    pinMode(led_pin, OUTPUT);

    digitalWrite(led_pin, HIGH);

    Serial.begin(115200);
    SerialNina.begin(115200);

    while (!digitalRead(button_start_pin) == HIGH)
        // Serial.println("Startup paused!");
        continue;

    Wire.begin(i2c_transmissionbus);
    Wire.onRequest(requestEvent);
}

void loop()
{

    // space for buttons for all the calibration things this is left for future adjustments

    //     if (command.equals("pause"))
    //         SerialNina.write('3');
    //     else if (command.equals("disconnect"))
    //         SerialNina.write('6');
    //     else if (command.equals("resetgain"))
    //         SerialNina.write('4');
    //     else if (command.equals("calibrate"))
    //         SerialNina.write('5');
    //     else if (command.equals("resume"))
    //         SerialNina.write('2');

    if (!startup_signal_sent && digitalRead(button_start_pin) == HIGH)
    {
        // Serial.println("Startup started!");
        startup_signal_sent = true;
        SerialNina.write(START_SIGNAL);
    }

    if (SerialNina.available())
    {
        if (startup)
        {
            SerialNina.readBytes(message_values, 8);

            if (message_values[0] == STARTUP_END)
            {
                startup = false;
                // flushing buffer
                while (SerialNina.available() > 0)
                {
                    char _ = SerialNina.read();
                }
                // Serial.println("Startup finished!");
            }
            else
            {
                // printFSRData(message_values);
                continue;
            }
        }
        else
        {
            SerialNina.readBytes(message_values, 8);
            update_centroid_distances(message_values);
            // printFSRData(message_values);
        }
    }
}

BLA::Matrix<num_centroids> distance_calc(BLA::Matrix<num_principal_components> pc_selection)
{
    BLA::Matrix<num_centroids> distance;
    for (int i = 0; i < centroids.Rows; i++)
    {
        // this part calculates the distance and is currently hardcoded for two principle principal components it would need to be adjusted for more PC
        BLA::Matrix<1, 2> centroid = centroids.Submatrix<1, 2>(i, 0);
        distance(i) = ((centroid(0) - pc_selection(0)) * (centroid(0) - pc_selection(0))) + ((centroid(0, 1) - pc_selection(0, 1)) * (centroid(0, 1) - pc_selection(0, 1)));
    }
    Serial << "pc_selection: " << pc_selection << '\n';
    return distance;
}

// convert the array into a matrix format
void convert_msgToMatrix(uint8_t *message_values)
{
    message_matrix = {message_values[0],
                      message_values[1],
                      message_values[2],
                      message_values[3],
                      message_values[4],
                      message_values[5],
                      message_values[6],
                      message_values[7]};
}

// print the original values recieved to Serial
void printFSRData(uint8_t message[])
{
    for (int i = 0; i < 8; i++)
    {
        Serial.print(message[i]);
        Serial.print('\t');
    }
    Serial.print("\r\n");
}

// function to start the update for the latest centroid distances, called when a new transmission is received
void update_centroid_distances(uint8_t *message_values)
{
    // convert to matrix format
    convert_msgToMatrix(message_values);

    // apply the PCA mean shift and the PCA cofficent matrix to the message to we can calculate distances in 2D space
    BLA::Matrix<8> transformed_message = coeff * (message_matrix - mu);
    BLA::Matrix<num_principal_components> pc_selection = transformed_message.Submatrix<num_principal_components, 1>(0, 0);

    // use the distance calculation formula
    BLA::Matrix<num_centroids> distance_matrix = distance_calc(pc_selection);

    // initialize the struct with values for transmission
    fakebuffer[0] = distance_matrix(0);
    fakebuffer[1] = distance_matrix(1);
    fakebuffer[2] = distance_matrix(2);

    Serial << "distance matrix:" << distance_matrix << '\n';
}

void requestEvent()
{
    for (int i = 0; i < 3; i++)
    {
        split.to_float = fakebuffer[i];
        for (int j = 0; j < 4; j++)
        {
            buff_I2C[(4 * i) + j] = split.to_byte[j];
        }
    }

    Wire.write(buff_I2C, 12);
}
