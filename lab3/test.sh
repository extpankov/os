echo
echo "=== Тест 1: обычный запуск (ожидаем PID/PPID, завершение ребёнка) ==="
./myfork

echo
echo "=== Тест 2: SIGINT (Ctrl+C) ==="
./myfork &
PID=$!
sleep 1
kill -INT $PID
wait $PID

echo
echo "=== Тест 3: SIGTERM ==="
./myfork &
PID=$!
sleep 1
kill -TERM $PID
wait $PID

echo
echo "=== Все тесты завершены ==="