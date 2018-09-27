{ stdenv, Literate }:

stdenv.mkDerivation rec {
  name = "waffle-${version}";
  version = "NaV";

  src = ./.;

  buildInputs = [ Literate ];

  makeFlags = [ "PREFIX=$(out)" ];

  meta = with stdenv.lib; {
    description = "A fully manual tiling wm";
    homepage = https://github.com/buffet/wmaffle;
    license = licenses.mpl20;
    platforms = platforms.unix;
  };
}
