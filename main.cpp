#include <iostream>
#include <string>
#include <string_view>
#include <array>
#include <cstdlib>      // for std::rand() and std::srand()
#include <ctime>        // for std::time()

// Generate a random number between min and max (inclusive)
// Assumes std::srand() has already been called
// Assumes max - min <= RAND_MAX
int getRandomNumber(int min, int max)
{
  static constexpr double fraction{ 1.0 / (RAND_MAX + 1.0) };
  // static used for efficiency, so we only calculate this value once
  // evenly distribute the random number across our range
  return min + static_cast<int>((max - min + 1) * (std::rand() * fraction));
}

class Player;
class Monster;

class Creature
{
protected:
    std::string m_name;
    char m_symbol;
    int m_health;
    int m_damage;
    int m_gold;

public:
    Creature(const std::string& name, char symbol, int health,
    int damage, int gold):
        m_name{ name },
        m_symbol{ symbol },
        m_health{ health },
        m_damage{ damage },
        m_gold{ gold }
    {
    }

    const std::string& getName() const { return m_name; }
    char getSymbol() const { return m_symbol; }
    int getHealth() const { return m_health; }
    int getDamage() const { return m_damage; }
    int getGold() const { return m_gold; }

    void reduceHealth(int health) { m_health -= health; }
    bool isDead() const { return m_health <= 0; }
    void addGold(int gold) { m_gold += gold; }

};

class Player : public Creature
{
private:
    int m_level{ 1 };

public:
    Player(const std::string& name):
        Creature{ name, '@', 10, 1, 0 }
    {
    }

    void levelUp()
    {
        ++m_level;
        ++m_damage;
    }

    int getLevel() const { return m_level; }
    bool hasWon() const { return m_level >= 20; }
    bool hasLost() const { return m_health <= 0; }

    friend void attackMonster(Player &player, Monster &monster);
    friend void attackPlayer(Player &player, Monster &monster);
    
};

class Monster : public Creature
{
public:
    enum Type
    {
        DRAGON,
        ORC,
        SLIME,
        MAX_TYPES
    };

private:
    struct MonsterData
     {
         std::string_view name;
         char symbol;
         int health;
         int damage;
         int gold;
     }; 

    static constexpr std::array<MonsterData, Monster::MAX_TYPES> 
        monsterData{{
            { "dragon", 'D', 20, 4, 100 },
            { "orc", 'o', 4, 2, 25 },
            { "slime", 's', 1, 1, 10 }
        }};

public:
    Monster(Type type)
        : Creature{ 
            monsterData[type].name.data(),
            monsterData[type].symbol,
            monsterData[type].health,
            monsterData[type].damage,
            monsterData[type].gold
        }
    {
    }

    static Type getRandomMonster()
    {
        return static_cast<Type>(getRandomNumber(0, MAX_TYPES - 1));
    }

    friend void attackMonster(Player &player, Monster &monster);
    friend void attackPlayer(Player &player, Monster &monster);
    
};

void printTable(const Player &player, const Monster &monster)
{
    std::cout << "________________________________________"
                << "________________________________________\n";

    std::cout   << "|Player:" << player.getName() << "|\t"
                << "|Health:" << player.getHealth() << "|\t"
                << "|Gold:" << player.getGold() << "|\t"
                << "|Damage:" << player.getDamage() << "|\t"
                << "|Level:" << player.getLevel() << "|";

    std::cout << "\n----------------------------------------"
                << "----------------------------------------\n";

    std::cout   << "|Monster:" << monster.getName() << "|\t"
                << "|Health:" << monster.getHealth() << "|\t"
                << "|Gold:" << monster.getGold() << "|\t"
                << "|Damage:" << monster.getDamage() << "|\t"
                << "|Symbol:" << monster.getSymbol() << "|";

    std::cout << "\n````````````````````````````````````````"
                << "````````````````````````````````````````\n";
}

void attackPlayer(Player &player, Monster &monster)
{
    // the player's health is reduced by the monster's damage.
    player.m_health -= monster.getDamage();
    std::cout << "The " << monster.getName() << " hit you for " <<
                monster.getDamage() << " damage.\n";
}

void attackMonster(Player &player, Monster &monster)
{
    // player attacks first
    // monsters health is reduced by player's attack
    monster.m_health -= player.getDamage();
    std::cout << "you hit the " << monster.getName() << 
            " for " << player.getDamage() << " damage.\n";

    // if monster dies, the player takes any gold the mosnter is carrying.
    // the player also levels up. increasing their level and damage by 1.
    if (monster.getHealth() <= 0)
    {
        std::cout << "You killed the " << monster.getName() << ".\n";

        player.levelUp();
        std::cout << "You are now level " << player.getLevel() << ".\n";

        player.m_gold += monster.getGold();
        std::cout << "You found " << monster.getGold() << " gold.\n";
        monster.m_gold = 0;
    } else {
        // if the monster does not die, the monster attacks the player back.
        attackPlayer(player, monster);
    }
}

// returns true if player successfully ran from this monster false otherwise
bool fightMonster(Player &player)
{

    // the player encounters one randomly generated monster
    Monster monster{ Monster::getRandomMonster() };

    std::cout << "\n[You have encountered a/an ";
    switch(monster.getSymbol())
    {
        case 'D':
            std::cout << "dragon (D).]\n";    break;
        case 'o':
            std::cout << "orc (o).]\n";      break;
        case 's':
            std::cout << "slime (s).]\n";    break;
    }
    // print initial table
    printTable(player, monster);

    // for each monster, the player has two choices: 
    // run or fight
    while (true)
    {
        std::cout << "(R)un or (F)ight: ";
        char choice{};
        std::cin >> choice;

        if (choice == 'f')
        {
            attackMonster(player, monster);
        }
        else if (choice == 'r')
        {
            // if the player decides to run, they have a 50 %
            // chance of escaping
            if (getRandomNumber(0, 1)) {
                // if the player escapes, they move to the next
                // encounter with no ill effects
                std::cout << "You escaped the " << monster.getName() << ".\n";
                return true;
            } else {
                // if the player does not escape, the monster
                // gets a free attack, 
                std::cout << "You couldn't escaped the " << monster.getName()
                    << ".\n";
                attackPlayer(player, monster);
            }
        }

        // if player is dead
        if (player.getHealth() <= 0)
            break;

        // if this monster is dead
        if (monster.getHealth() <= 0)
            break;
    }

    return false;
}

int main()
{
    std::srand(static_cast<unsigned int>(std::time(nullptr))); 
    // set initial seed value to system clock
    std::rand();
    // get rid of first result
 
    std::cout << "Enter your name: ";
    std::string playerName{};
    std::cin >> playerName;
    std::cout << "Welcome, " << playerName << '\n';

    // make a player
    Player player{ playerName };

    while (true)
    {
        if (fightMonster(player))
        {
            // if you choose run and got lucky
            // skip this monster
            continue;
        }

        // the game ends when the player has died (loss) or 
        // reached level 20 (win)

        // if the player dies
        if (player.getHealth() <= 0) {
            std::cout << "You died at level " << player.getLevel()
                << " and with " << player.getGold() << " gold.\n";

            // end the game
            break;
        }

        // if the player wins
        if (player.hasWon()) {
            std::cout << "You Won! you had " << player.getGold()
            << " gold.\n";

            // end the game
            break;
        }
    }

    return 0;
}
