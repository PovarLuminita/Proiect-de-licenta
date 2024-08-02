using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using EasyModbus;
using System.Threading;

namespace Client_TCP
{
    public partial class Form1 : Form
    {
        private EasyModbus.ModbusClient modbusClient;
        int[] serverResponse = new int[16];
        private AnimatedLed[] animatedLeds; // array de obiecte pentru cele 16 led-uri

        public Form1()
        {
            InitializeComponent();
            modbusClient = new EasyModbus.ModbusClient();
            InitializeLeds();
        }

        // Buton pentru conectare
        private void btnConnect_Click(object sender, EventArgs e)
        {
            try
            {
                modbusClient.IPAddress = txtIP.Text;  // Seteaza adresa IP a serverului
                modbusClient.UnitIdentifier = 20;
                modbusClient.Port = 502;
                modbusClient.SerialPort = null;

                modbusClient.Connect();  // Conectare la server

                // Verifica daca conexiunea a fost realizata
                if (modbusClient.Connected)
                {
                    lblStatus.Text = "Connected to Server";
                    lblStatus.BackColor = Color.LightGreen;
                    btnDisconnect.Enabled = true;
                }
            }
            catch (Exception exc)
            {
                MessageBox.Show(exc.Message, "Unable to connect to Server", MessageBoxButtons.OK, MessageBoxIcon.Error);
            }
        }

        // Buton pentru deconectare
        private void btnDisconnect_Click(object sender, EventArgs e)
        {
            modbusClient.Disconnect();
            btnDisconnect.Enabled = false;
            lblStatus.Text = "Disconnected from server";
            lblStatus.BackColor = Color.LightPink;
        }

        // Buton pentru citirea registrilor
        private void btnReadHoldingRegisters_Click(object sender, EventArgs e)
        {
            try
            {
                serverResponse = modbusClient.ReadHoldingRegisters(0, 16);

                text1.Text = serverResponse[0].ToString();
                text2.Text = serverResponse[1].ToString();
                text3.Text = serverResponse[2].ToString();
                text4.Text = serverResponse[3].ToString();
                text5.Text = serverResponse[4].ToString();
                text6.Text = serverResponse[5].ToString();
                text7.Text = serverResponse[6].ToString();
                text8.Text = serverResponse[7].ToString();
                text9.Text = serverResponse[8].ToString();
                text10.Text = serverResponse[9].ToString();
                text11.Text = serverResponse[10].ToString();
                text12.Text = serverResponse[11].ToString();
                text13.Text = serverResponse[12].ToString();
                text14.Text = serverResponse[13].ToString();
                text15.Text = serverResponse[14].ToString();
                text16.Text = serverResponse[15].ToString();
                UpdateLedColors(); // functie care face update la culorile led-urilor din aplicatie
            }
            catch (Exception exc)
            {
                MessageBox.Show(exc.Message, "Exception Reading values from Server", MessageBoxButtons.OK, MessageBoxIcon.Error);
            }
        }


        private void InitializeLeds()
        {
            animatedLeds = new AnimatedLed[16];

            animatedLeds[0] = a1;
            animatedLeds[1] = a2;
            animatedLeds[2] = a3;
            animatedLeds[3] = a4;
            animatedLeds[4] = a5;
            animatedLeds[5] = a6;
            animatedLeds[6] = a7;
            animatedLeds[7] = a8;
            animatedLeds[8] = a9;
            animatedLeds[9] = a10;
            animatedLeds[10] = a11;
            animatedLeds[11] = a12;
            animatedLeds[12] = a13;
            animatedLeds[13] = a14;
            animatedLeds[14] = a15;
            animatedLeds[15] = a16;
        }


        // Functie care face update la culorile de la led-uri in functie de comanda de citire primita de la gateway 
        private void UpdateLedColors()
        {
            for (int i = 0; i < animatedLeds.Length; i++)
            {
                int value = serverResponse[i];
                animatedLeds[i].BackColor = GetColorFromValue(value);
                animatedLeds[i].Invalidate();  // pentru a afisa schimbarile din nou
            }
        }

        // Functie care alege intensitatea culorii pentru fiecare led in functie de fiecare valoare a pwm-ului
        private Color GetColorFromValue(int value)
        {
            if (value >= 1 && value <= 20)
                return Color.FromArgb(255, 192, 192);
            else if (value >= 21 && value <= 40)
                return Color.FromArgb(255, 128, 128);
            else if (value >= 41 && value <= 60)
                return Color.Red;
            else if (value >= 61 && value <= 80)
                return Color.FromArgb(192, 0, 0);
            else if (value >= 81 && value < 100)
                return Color.Maroon;
            else
                return Color.Silver;
        }

        // Buton pentru citirea statusului bobinelor
        private void button_read_status_Click(object sender, EventArgs e)
        {
            try
            {
                bool[] ResponseCoils = modbusClient.ReadCoils(0, 16);

                if (ResponseCoils[0] == true) { l1.Text = "ON"; } else { l1.Text = "OFF"; }
                if (ResponseCoils[1] == true) { l2.Text = "ON"; } else { l2.Text = "OFF"; }
                if (ResponseCoils[2] == true) { l3.Text = "ON"; } else { l3.Text = "OFF"; }
                if (ResponseCoils[3] == true) { l4.Text = "ON"; } else { l4.Text = "OFF"; }
                if (ResponseCoils[4] == true) { l5.Text = "ON"; } else { l5.Text = "OFF"; }
                if (ResponseCoils[5] == true) { l6.Text = "ON"; } else { l6.Text = "OFF"; }
                if (ResponseCoils[6] == true) { l7.Text = "ON"; } else { l7.Text = "OFF"; }
                if (ResponseCoils[7] == true) { l8.Text = "ON"; } else { l8.Text = "OFF"; }
                if (ResponseCoils[8] == true) { l9.Text = "ON"; } else { l9.Text = "OFF"; }
                if (ResponseCoils[9] == true) { l10.Text = "ON"; } else { l10.Text = "OFF"; }
                if (ResponseCoils[10] == true) { l11.Text = "ON"; } else { l11.Text = "OFF"; }
                if (ResponseCoils[11] == true) { l12.Text = "ON"; } else { l12.Text = "OFF"; }
                if (ResponseCoils[12] == true) { l13.Text = "ON"; } else { l13.Text = "OFF"; }
                if (ResponseCoils[13] == true) { l14.Text = "ON"; } else { l14.Text = "OFF"; }
                if (ResponseCoils[14] == true) { l15.Text = "ON"; } else { l15.Text = "OFF"; }
                if (ResponseCoils[15] == true) { l16.Text = "ON"; } else { l16.Text = "OFF"; }
                if (ResponseCoils[0] == true) { animatedLed1.BackColor = Color.DarkRed; } else { animatedLed1.BackColor = Color.Silver; }
                if (ResponseCoils[1] == true) { animatedLed2.BackColor = Color.DarkRed; } else { animatedLed2.BackColor = Color.Silver; }
                if (ResponseCoils[2] == true) { animatedLed3.BackColor = Color.DarkRed; } else { animatedLed3.BackColor = Color.Silver; }
                if (ResponseCoils[3] == true) { animatedLed4.BackColor = Color.DarkRed; } else { animatedLed4.BackColor = Color.Silver; }
                if (ResponseCoils[4] == true) { animatedLed5.BackColor = Color.DarkRed; } else { animatedLed5.BackColor = Color.Silver; }
                if (ResponseCoils[5] == true) { animatedLed6.BackColor = Color.DarkRed; } else { animatedLed6.BackColor = Color.Silver; }
                if (ResponseCoils[6] == true) { animatedLed7.BackColor = Color.DarkRed; } else { animatedLed7.BackColor = Color.Silver; }
                if (ResponseCoils[7] == true) { animatedLed8.BackColor = Color.DarkRed; } else { animatedLed8.BackColor = Color.Silver; }
                if (ResponseCoils[8] == true) { animatedLed9.BackColor = Color.DarkRed; } else { animatedLed9.BackColor = Color.Silver; }
                if (ResponseCoils[9] == true) { animatedLed10.BackColor = Color.DarkRed; } else { animatedLed10.BackColor = Color.Silver; }
                if (ResponseCoils[10] == true) { animatedLed11.BackColor = Color.DarkRed; } else { animatedLed11.BackColor = Color.Silver; }
                if (ResponseCoils[11] == true) { animatedLed12.BackColor = Color.DarkRed; } else { animatedLed12.BackColor = Color.Silver; }
                if (ResponseCoils[12] == true) { animatedLed13.BackColor = Color.DarkRed; } else { animatedLed13.BackColor = Color.Silver; }
                if (ResponseCoils[13] == true) { animatedLed14.BackColor = Color.DarkRed; } else { animatedLed14.BackColor = Color.Silver; }
                if (ResponseCoils[14] == true) { animatedLed15.BackColor = Color.DarkRed; } else { animatedLed15.BackColor = Color.Silver; }
                if (ResponseCoils[15] == true) { animatedLed16.BackColor = Color.DarkRed; } else { animatedLed16.BackColor = Color.Silver; }
            }
            catch (Exception exc)
            {
                MessageBox.Show(exc.Message, "Exception Reading values from Server", MessageBoxButtons.OK, MessageBoxIcon.Error);
            }
        }

        private void animatedLed2_Click(object sender, EventArgs e)
        {

        }
    }
}
