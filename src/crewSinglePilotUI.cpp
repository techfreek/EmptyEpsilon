#include "crewSinglePilotUI.h"

CrewSinglePilotUI::CrewSinglePilotUI()
{
    tube_load_type = MW_None;
    jump_distance = 1.0;
}

void CrewSinglePilotUI::onCrewUI()
{
    float radarDistance = 5000;
    sf::Vector2f mouse = InputHandler::getMousePos();
    sf::Vector2f radar_center = getWindowSize() / 2.0f;
    radar_center.x /= 2.0f;
    float radar_size = radar_center.x - 20;

    if (InputHandler::mouseIsReleased(sf::Mouse::Left))
    {
        sf::Vector2f diff = mouse - radar_center;
        if (sf::length(diff) < radar_size)
        {
            P<SpaceObject> target;
            sf::Vector2f mousePosition = my_spaceship->getPosition() + diff / radar_size * radarDistance;
            PVector<Collisionable> list = CollisionManager::queryArea(mousePosition - sf::Vector2f(50, 50), mousePosition + sf::Vector2f(50, 50));
            foreach(Collisionable, obj, list)
            {
                P<SpaceObject> spaceObject = obj;
                if (spaceObject && spaceObject->canBeTargetedByPlayer() && spaceObject != my_spaceship)
                {
                    if (!target || sf::length(mousePosition - spaceObject->getPosition()) < sf::length(mousePosition - target->getPosition()))
                        target = spaceObject;
                }
            }
            if (target)
                my_spaceship->commandSetTarget(target);
            else
                my_spaceship->commandTargetRotation(sf::vector2ToAngle(diff));
        }
    }

    drawRadar(radar_center, radar_size, radarDistance, false, my_spaceship->getTarget(), sf::FloatRect(0, 0, getWindowSize().x / 2.0f, 900));

    keyValueDisplay(sf::FloatRect(10, 30, 200, 20), 0.5, "Energy", string(int(my_spaceship->energy_level)), 20);
    keyValueDisplay(sf::FloatRect(10, 50, 200, 20), 0.5, "Hull", string(int(my_spaceship->hull_strength * 100 / my_spaceship->hull_max)), 20);
    keyValueDisplay(sf::FloatRect(10, 70, 200, 20), 0.5, "Shields", string(int(100 * my_spaceship->front_shield / my_spaceship->front_shield_max)) + "/" + string(int(100 * my_spaceship->rear_shield / my_spaceship->rear_shield_max)), 20);
    if (my_spaceship->front_shield_max > 0 || my_spaceship->rear_shield_max > 0)
    {
        if (toggleButton(sf::FloatRect(10, 90, 170, 25), my_spaceship->shields_active, my_spaceship->shields_active ? "Shields:ON" : "Shields:OFF", 20))
            my_spaceship->commandSetShields(!my_spaceship->shields_active);
    }
    dockingButton(sf::FloatRect(10, 115, 170, 25), 20);

    impulseSlider(sf::FloatRect(10, 650, 40, 200), 15);
    float x = 60;
    if (my_spaceship->hasWarpdrive)
    {
        warpSlider(sf::FloatRect(x, 650, 40, 200), 15);
        x += 50;
    }
    if (my_spaceship->hasJumpdrive)
    {
        jumpSlider(jump_distance, sf::FloatRect(x, 650, 40, 200), 15);
        jumpButton(jump_distance, sf::FloatRect(x, 865, 80, 30), 20);
        x += 50;
    }

    if (my_spaceship->weaponTubes > 0)
    {
        float y = 900 - 5;
        for(int n=0; n<my_spaceship->weaponTubes; n++)
        {
            y -= 30;
            weaponTube(tube_load_type, n, sf::FloatRect(getWindowSize().x / 2.0 - 100, y, 100, 30), sf::FloatRect(getWindowSize().x / 2.0 - 300, y, 200, 30), 20);
        }

        for(int n=0; n<MW_Count; n++)
        {
            if (my_spaceship->weapon_storage_max[n] > 0)
            {
                y -= 25;
                if (toggleButton(sf::FloatRect(getWindowSize().x / 2.0 - 150, y, 150, 25), tube_load_type == n, getMissileWeaponName(EMissileWeapons(n)) + " x" + string(my_spaceship->weapon_storage[n]), 20))
                {
                    if (tube_load_type == n)
                        tube_load_type = MW_None;
                    else
                        tube_load_type = EMissileWeapons(n);
                }
            }
        }
    }

    if (my_spaceship->getTarget())
    {
        P<SpaceObject> target = my_spaceship->getTarget();
        float distance = sf::length(target->getPosition() - my_spaceship->getPosition());
        float heading = sf::vector2ToAngle(target->getPosition() - my_spaceship->getPosition());
        if (heading < 0) heading += 360;
        text(sf::FloatRect(getWindowSize().x / 2.0 - 100, 50, 100, 20), target->getCallSign(), AlignRight, 20);
        text(sf::FloatRect(getWindowSize().x / 2.0 - 100, 70, 100, 20), "Distance: " + string(distance / 1000.0, 1) + "km", AlignRight, 20);
        text(sf::FloatRect(getWindowSize().x / 2.0 - 100, 90, 100, 20), "Heading: " + string(int(heading)), AlignRight, 20);

        P<SpaceShip> ship = target;
        if (ship && ship->scanned_by_player == SS_NotScanned)
        {
            if (my_spaceship->scanning_delay > 0.0)
            {
                progressBar(sf::FloatRect(getWindowSize().x / 2.0 - 100, 110, 100, 20), my_spaceship->scanning_delay, 8.0, 0.0);
            }else{
                if (button(sf::FloatRect(getWindowSize().x / 2.0 - 100, 110, 100, 30), "Scan", 20))
                    my_spaceship->commandScan(target);
            }
        }else{
            text(sf::FloatRect(getWindowSize().x / 2.0 - 100, 110, 100, 20), factionInfo[target->faction_id]->name, AlignRight, 20);
            if (ship && ship->ship_template)
            {
                text(sf::FloatRect(getWindowSize().x / 2.0 - 100, 130, 100, 20), ship->ship_template->name, AlignRight, 20);
                text(sf::FloatRect(getWindowSize().x / 2.0 - 100, 150, 100, 20), "Shields: " + string(int(ship->front_shield)) + "/" + string(int(ship->rear_shield)), AlignRight, 20);
            }
        }
        P<SpaceStation> station = target;
        if (station)
        {
            text(sf::FloatRect(getWindowSize().x / 2.0 - 100, 150, 100, 20), "Shields: " + string(int(station->shields)), AlignRight, 20);
        }
    }

    if (my_spaceship->comms_state == CS_ChannelOpenPlayer)
    {
        std::vector<string> lines = my_spaceship->comms_incomming_message.split("\n");
        float y = 100;
        static const unsigned int max_lines = 20;
        for(unsigned int n=lines.size() > max_lines ? lines.size() - max_lines : 0; n<lines.size(); n++)
        {
            text(sf::FloatRect(getWindowSize().x / 2.0 + 20, y, 600, 30), lines[n]);
            y += 30;
        }
        y += 30;
        comms_player_message = textEntry(sf::FloatRect(820, y, 450, 50), comms_player_message);
        if (button(sf::FloatRect(getWindowSize().x - 330, y, 110, 50), "Send") || InputHandler::keyboardIsPressed(sf::Keyboard::Return))
        {
            my_spaceship->commandSendCommPlayer(comms_player_message);
            comms_player_message = "";
        }

        if (button(sf::FloatRect(getWindowSize().x / 2.0 + 20, 800, 300, 50), "Close channel"))
            my_spaceship->commandCloseTextComm();
    }else{
        switch(my_spaceship->main_screen_setting)
        {
        case MSS_LongRange:
            drawRadar(sf::Vector2f(getWindowSize().x / 4 * 3, 450), radar_size, 50000, true, NULL, sf::FloatRect(getWindowSize().x / 2.0f, 0, getWindowSize().x / 2.0f, 900));
            break;
        case MSS_Tactical:
            drawRadar(sf::Vector2f(getWindowSize().x / 4 * 3, 450), radar_size, 5000, false, NULL, sf::FloatRect(getWindowSize().x / 2.0f, 0, getWindowSize().x / 2.0f, 900));
            break;
        default:
            draw3Dworld(sf::FloatRect(getWindowSize().x / 2.0f, 0, getWindowSize().x / 2.0f, 900));
            break;
        }
    }
}