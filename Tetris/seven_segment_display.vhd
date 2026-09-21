library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.STD_LOGIC_ARITH.ALL;
use IEEE.STD_LOGIC_UNSIGNED.ALL;

entity seven_segment_display is
    Port (
        clk : in STD_LOGIC;  -- Clock signal
        digit : in STD_LOGIC_VECTOR(3 downto 0);  -- 4-bit input for Hex digit (0-F)
        segments : out STD_LOGIC_VECTOR(6 downto 0)  -- Outputs (g,f,e,d,c,b,a)
    );
end seven_segment_display;

architecture RTL of seven_segment_display is
    signal seg_reg : STD_LOGIC_VECTOR(6 downto 0);  -- Register for storing segment data
begin
    -- Sequential logic to update the display register on every clock edge
    process(clk)
    begin
        if rising_edge(clk) then
            case digit is
                -- Cifre 0 - 9
                when "0000" => seg_reg <= "1000000"; -- 0 (a,b,c,d,e,f)
                when "0001" => seg_reg <= "1111001"; -- 1 (b,c)
                when "0010" => seg_reg <= "0100100"; -- 2 (a,b,d,e,g)
                when "0011" => seg_reg <= "0110000"; -- 3 (a,b,c,d,g)
                when "0100" => seg_reg <= "0011001"; -- 4 (b,c,f,g)
                when "0101" => seg_reg <= "0010010"; -- 5 (a,c,d,f,g)
                when "0110" => seg_reg <= "0000010"; -- 6 (a,c,d,e,f,g)
                when "0111" => seg_reg <= "1111000"; -- 7 (a,b,c)
                when "1000" => seg_reg <= "0000000"; -- 8 (svi ukljuceni)
                when "1001" => seg_reg <= "0010000"; -- 9 (a,b,c,d,f,g)

                -- Hex cifre A - F
                when "1010" => seg_reg <= "0001000"; -- A (a,b,c,e,f,g)
                when "1011" => seg_reg <= "0000011"; -- b (c,d,e,f,g) - malo b radi prepoznavanja
                when "1100" => seg_reg <= "1000110"; -- C (a,d,e,f)
                when "1101" => seg_reg <= "0100001"; -- d (b,c,d,e,g) - malo d radi prepoznavanja
                when "1110" => seg_reg <= "0000110"; -- E (a,d,e,f,g)
                when "1111" => seg_reg <= "0001110"; -- F (a,e,f,g)

                when others => seg_reg <= "1111111"; -- Svi segmenti iskljuceni
            end case;
        end if;
    end process;

    -- Combinational logic to drive the segments
    segments <= seg_reg;
    
end RTL;