library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

-- Operacije ALU jedinice:
--     op        |        Rezultat        
--        000    |        A and B        
--        001    |        A or B        
--        010    |        A xor B        
--        011    |        A + B - Cin       
--        100    |        A - B - Cin    
--        101    |        A << B (logicki shift u levo)
--        110    |        A >> B (logicki shift u desno)           
--        111    |        A >> B (aritmeticki shift u desno)

entity ALUX is
    generic (
        size : natural := 16
    );
    port (
        A    : in  std_logic_vector(size-1 downto 0);
        B    : in  std_logic_vector(size-1 downto 0);
        Cin  : in  std_logic;
        op   : in  std_logic_vector(2 downto 0);
        F    : out std_logic_vector(size-1 downto 0);
        Cout : out std_logic
    );
end entity;

architecture rtl of ALUX is
begin

    process(A, B, Cin, op) is 
        variable ta, tb, tf : std_logic_vector(size downto 0);
        variable shift_amt  : integer range 0 to size;
    begin 
        -- Proširenje za pretek (Carry)
        ta(size) := A(size-1);
        ta(size-1 downto 0) := A(size-1 downto 0);
        
        tb(size) := B(size-1);
        tb(size-1 downto 0) := B(size-1 downto 0);
        
        -- Pomeraj (shift) se računa iz celobrojne vrednosti ulaza B
        shift_amt := to_integer(unsigned(B));

        case op is
            -- Logičke operacije
            when "000" => 
                F <= A and B; 
                Cout <= '0';

            when "001" => 
                F <= A or B; 
                Cout <= '0';

            when "010" => 
                F <= A xor B; 
                Cout <= '0';

            -- Sabiranje: A + B - Cin
           when "011" => 
					 if Cin = '0' then
						  tf := std_logic_vector(resize(unsigned(A), size+1) + unsigned(B));
					 else
						  tf := std_logic_vector(resize(unsigned(A), size+1) + unsigned(B) - 1);
					 end if;
					 
					 F <= tf(size-1 downto 0);
					 Cout <= tf(size);

            -- Oduzimanje: A - B - Cin
            when "100" => 
                if Cin = '0' then
                    tf := std_logic_vector(to_unsigned(to_integer(unsigned(ta)) - to_integer(unsigned(tb)), size+1));
                else
                    tf := std_logic_vector(to_unsigned(to_integer(unsigned(ta)) - to_integer(unsigned(tb)) - 1, size+1));
                end if;
                F <= tf(size-1 downto 0);
                Cout <= tf(size);

            -- Logički shift ulevo (A << B)
            when "101" => 
                if shift_amt < size then
                    F <= std_logic_vector(shift_left(unsigned(A), shift_amt));
                else
                    F <= (others => '0'); -- Ako je shift veći ili jednak širini magistrale
                end if;
                Cout <= '0';

            -- Logički shift udesno (A >> B)
            when "110" => 
                if shift_amt < size then
                    F <= std_logic_vector(shift_right(unsigned(A), shift_amt));
                else
                    F <= (others => '0');
                end if;
                Cout <= '0';

            -- Aritmetički shift udesno (A >> B) - čuva znak (MSB)
            when "111" => 
                if shift_amt < size then
                    F <= std_logic_vector(shift_right(signed(A), shift_amt));
                else
                    F <= (others => A(size-1)); -- Popunjava bitom znaka ako je shift prevelik
                end if;
                Cout <= '0';

            when others =>
                F <= (others => '0');
                Cout <= '0';
        end case;
    end process; 

end rtl;