library IEEE;
use IEEE.STD_LOGIC_1164.ALL;

entity pulse_gen is
    Port ( clk    : in  STD_LOGIC;
           enable : in  STD_LOGIC;
           pulse  : out STD_LOGIC);
end pulse_gen;

architecture Behavioral of pulse_gen is
    signal enable_d : STD_LOGIC := '0';
begin
    process(clk)
    begin
        if rising_edge(clk) then
            enable_d <= enable;
            if (enable = '1' and enable_d = '0') then
                pulse <= '1'; -- Generates a 1-clock pulse on rising edge of enable
            else
                pulse <= '0';
            end if;
        end if;
    end process;
end Behavioral;
