/**
 * @brief
 *
 */

void onDmxFrame(uint16_t universe, uint16_t length, uint8_t sequence, uint8_t *data, IPAddress remoteIP)
{
    sendFrame = 1;
    // Set brightness ofthe whole strip, for all the strips, if they are enabled
    if (universe == 15)
    {
        for (byte i = 0; i < totalLEDStrips; i++)
        {
            if (stripsEnabled[i])
            {
                strips[i].setBrightness(data[0]);
                strips[i].show();
            }
        }
    }

    // Store which universe has got in
    if ((universe - startUniverse) < maxUniverses)
    {
        universesReceived[universe - startUniverse] = 1;
    }

    for (int i = 0; i < maxUniverses; i++)
    {
        if (universesReceived[i] == 0)
        {
            //      logln("Broke");
            sendFrame = 0;
            break;
        }
    }

    for (int i = 0; i < length / channelsPerLed; i++)
    {
        int led = i + (universe - startUniverse) * (previousDataLength / channelsPerLed);

        if (led < numLeds)
        {
            if (channelsPerLed == 4)
            {
                // -- For RGBW or GRBW type strips -- //
                for (byte s = 0; s < totalLEDStrips; s++)
                {
                    // Go through all the led strips ..
                    if (stripsEnabled[s])
                    {
                        // that are enabled..
                        // and set the ccolor to incoming data
                        strips[s].setPixelColor(led, data[i * channelsPerLed], data[i * channelsPerLed + 1], data[i * channelsPerLed + 2], data[i * channelsPerLed + 3]);
                    }
                }
            }
            if (channelsPerLed == 3)
            {
                // -- For RGB or GRB type strips -- //
                for (byte s = 0; s < totalLEDStrips; s++)
                {
                    // Go through all the led strips ..
                    if (stripsEnabled[s])
                    {
                        // that are enabled..
                        // and set the ccolor to incoming data
                        strips[s].setPixelColor(led, data[i * channelsPerLed], data[i * channelsPerLed + 1], data[i * channelsPerLed + 2]);
                    }
                }
            }
        }
    }
    previousDataLength = length;

    if (sendFrame)
    {
        for (byte s = 0; s < totalLEDStrips; s++)
        {
            // Go through all the led strips ..
            if (stripsEnabled[s])
            {
                // that are enabled..
                strips[s].show();
            }
        }
        // Reset universeReceived to 0
        memset(universesReceived, 0, maxUniverses);
    }
}

void startArtnetMethods()
{
    artnet.setBroadcast(broadcast);
    artnet.setArtDmxCallback(onDmxFrame);
}

void readArtnet()
{
    artnet.read();
}